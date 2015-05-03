#include <cstring>
#include <vector>
#include <map>
#include <sys/queue.h>
#include <evhttp.h>
#include <mecab.h>
#include "Furigana.h"
#include "Server.h"

#define PARSE_URI(request, params) { \
    char const *uri = evhttp_request_uri(request); \
    evhttp_parse_query(uri, &params); \
}


#define PARAM_GET_STR(var, params, name, mendatory) { \
    var = evhttp_find_header(&params_get, name); \
    if (!var && mendatory) { \
        http_send(request, "<err message=\"field '" name "' is mendatory\"/>\n"); \
        return; \
    } \
}

inline static void output_xml_header(struct evbuffer *buffer) {
    evbuffer_add_printf(buffer, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    evbuffer_add_printf(buffer, "<root>\n");
}

inline static void output_xml_footer(struct evbuffer *buffer) {
    evbuffer_add_printf(buffer, "</root>\n");
}

inline static void kana_output_xml(const char *kana, struct evbuffer *buffer) {
    evbuffer_add_printf(buffer, "<kana><![CDATA[");
    evbuffer_add_printf(buffer, "%s", kana);
    evbuffer_add_printf(buffer, "]]></kana>\n");
} 

inline static void parse_output_xml_header(struct evbuffer *buffer) {
    evbuffer_add_printf(buffer, "<parse>\n");
}

inline static void parse_output_xml_footer(struct evbuffer *buffer) {
    evbuffer_add_printf(buffer, "</parse>\n");
}

inline static void cdata_output_xml(const char *string, struct evbuffer *buffer) {
    evbuffer_add_printf(buffer, "<![CDATA[%s]]>", string);
}

inline static void reading_output_xml(
    std::pair<std::string,
    std::string> text,
    struct evbuffer *buffer
) {
    evbuffer_add_printf(
        buffer,
        "<reading furigana=\"%s\"><![CDATA[%s]]></reading>",
        text.second.c_str(),
        text.first.c_str()
    );
}

inline static void token_output_xml_header(struct evbuffer *buffer) {
    evbuffer_add_printf(buffer, "<token>");
}

inline static void token_output_xml_footer(struct evbuffer *buffer) {
    evbuffer_add_printf(buffer, "</token>\n");
}

inline static void token_output_xml(const char *token, struct evbuffer *buffer) {
    evbuffer_add_printf(buffer, "<token><![CDATA[");
    evbuffer_add_printf(buffer, "%s",token);
    evbuffer_add_printf(buffer, "]]></token>\n");
}





/**
 *
 */
static void http_send(struct evhttp_request *request, const char *fmt, ...) {
    struct evbuffer *buffer = evbuffer_new();
    evhttp_add_header(request->output_headers, "Content-Type", "TEXT/XML; charset=UTF8");
    va_list ap;
    va_start(ap, fmt);
    evbuffer_add_vprintf(buffer, fmt, ap);
    va_end(ap);
    evhttp_send_reply(request, HTTP_OK, "", buffer);
    evbuffer_free(buffer);
}



/**** uri: *
 *
 */
static void http_callback_default(struct evhttp_request *request, void *data) {
    evhttp_send_error(request, HTTP_NOTFOUND, "Service not found");
}

/**** uri: /kana?str=*
 *
 */
static void http_kana_callback(struct evhttp_request *request, void *data) {

    //parse uri
    struct evkeyvalq params_get;
    PARSE_URI(request, params_get);


    //get "str"
    char const *str;
    PARAM_GET_STR(str, &params_get, "str", true);

    //we parse into kana
    Server* server = (Server*) data;
    const char *kana = server->yomiTagger->parse(str);

    //TODO add error handling

    //prepare output
    struct evbuffer *buffer = evbuffer_new();

    output_xml_header(buffer);
    std::string enforcedHiranagas = server->furigana.katakana_to_hiragana(kana);
    kana_output_xml(enforcedHiranagas.c_str(), buffer);
    output_xml_footer(buffer);

    //send
    evhttp_add_header(request->output_headers, "Content-Type", "TEXT/XML; charset=UTF8");

    evhttp_send_reply(request, HTTP_OK, "", buffer);

}

/**** uri: /parse?str=*
 *
 */
static void http_parse_callback(struct evhttp_request *request, void *data) {

    //parse uri
    struct evkeyvalq params_get;
    PARSE_URI(request, params_get);


    //get "str"
    char const *str;
    PARAM_GET_STR(str, &params_get, "str", true);

    //we parse into kana
    Server* server = (Server*) data;

    std::vector<std::string> tokens;
    const MeCab::Node* node = server->tagger->parseToNode(str);
    for (; node; node = node->next) {
        if (node->stat != MECAB_BOS_NODE && node->stat != MECAB_EOS_NODE) {
            tokens.push_back(std::string(node->surface, node->length));
        }
    }

    //TODO add error handling

    //prepare output
    struct evbuffer *buffer = evbuffer_new();

    output_xml_header(buffer);
    parse_output_xml_header(buffer);

    for (auto& oneToken : tokens) {
        token_output_xml(oneToken.c_str(), buffer);
    }

    parse_output_xml_footer(buffer);
    output_xml_footer(buffer);

    //send
    evhttp_add_header(request->output_headers, "Content-Type", "TEXT/XML; charset=UTF8");

    evhttp_send_reply(request, HTTP_OK, "", buffer);

}

static std::string mecab_node_get_reading(const MeCab::Node *node) {
#define MECAB_FEATURE_READING_FIELD 8
    size_t field = 0;
    char *token, *infos = strdupa(node->feature);

    token = strtok(infos, ",");
    while (token != NULL) {
      field++;
      if (field == MECAB_FEATURE_READING_FIELD) {
          return std::string(token);
      }
      token = strtok(NULL, ",");
    }
    return "";
}

/**** uri: /furigana?str=*
 *
 */
static void http_furigana_callback(struct evhttp_request *request, void *data) {

    //parse uri
    struct evkeyvalq params_get;
    PARSE_URI(request, params_get);


    //get "str"
    char const *str;
    PARAM_GET_STR(str, &params_get, "str", true);

    //we parse into furigana => token+kana 
    Server* server = (Server*) data;

    std::vector< std::vector<std::pair<std::string, std::string> > > tokens;
    const MeCab::Node* node = server->tagger->parseToNode(str); for (; node; node = node->next) {
        if (node->stat != MECAB_BOS_NODE && node->stat != MECAB_EOS_NODE) {
            std::string token(node->surface, node->length);
            std::string kana(mecab_node_get_reading(node));

            std::vector<std::pair<std::string, std::string> > smallerTokens;
            smallerTokens = server->furigana.tokenize(token, kana);
            tokens.insert(
                tokens.end(),
                smallerTokens
            );
        }
    }

    //TODO add error handling

    //prepare output
    struct evbuffer *buffer = evbuffer_new();

    output_xml_header(buffer);
    parse_output_xml_header(buffer);

    for (auto& readings : tokens) {
        token_output_xml_header(buffer);
        for (auto& text : readings) {
            if (text.second.empty()) {
                cdata_output_xml(text.first.c_str(), buffer);
            } else {
                reading_output_xml(text, buffer);
            }
        }
        token_output_xml_footer(buffer);
    }

    parse_output_xml_footer(buffer);
    output_xml_footer(buffer);

    //send
    evhttp_add_header(request->output_headers, "Content-Type", "TEXT/XML; charset=UTF8");

    evhttp_send_reply(request, HTTP_OK, "", buffer);

}





/**
 *
 */

Server::Server(std::string address, int port) {

    struct event_base *base = event_init();
    struct evhttp *server = evhttp_new(base);
    int res = evhttp_bind_socket(server, address.c_str(), port);

    wakatiTagger = MeCab::createTagger("-Owakati");
    yomiTagger = MeCab::createTagger("-Oyomi");
    tagger = MeCab::createTagger("");

    if (res != 0) {
        std::cout <<  "[ERROR] Could not start http server!" << std::endl;
        return;
    }

    evhttp_set_gencb(server, http_callback_default, this);
    evhttp_set_cb(server, "/kana", http_kana_callback, this);
    evhttp_set_cb(server, "/parse", http_parse_callback, this);
    evhttp_set_cb(server, "/furigana", http_furigana_callback, this);

    event_base_dispatch(base);
}

/**
 *
 *
 */
Server::~Server() {
    delete wakatiTagger;
    delete yomiTagger;
    delete tagger;

}

