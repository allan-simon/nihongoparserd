nihongoparserd
===========

A service to convert japanese languages into romaji, kana etc.

for the moment it's based on libmecab but it may be replaced by homebrew parser in the future

### Requirement ###

  * libexpat
  * libevent
  * libmecab2
  * cmake

a C++11 compatible compiler

### Usage ###

    ./nihonggoparserd -p PORT [-h HOSTNAME]

That will launch an HTTP server listening on port PORT, eventually HOSTNAME.
It provides the following API calls, that will return an XML answer.
Note that contents are wrapped into `<![CDATA[...]]>`, which is removed from the following examples for readability.

#### Parse ####

URL: `/parse?str=*`

Example of response for `/parse?str=学校は家から遠いの？`

```xml

<?xml version="1.0" encoding="UTF-8"?>
<root>
    <parse>
        <token>学校</token>
        <token>は</token>
        <token>家</token>
        <token>から</token>
        <token>遠い</token>
        <token>の</token>
        <token>？</token>
    </parse>
</root>

```

#### Furigana ####

URL: `/furigana?str=*`

Example of response for `/furigana?str=学校は家から遠いの？`

```xml

<?xml version="1.0" encoding="UTF-8"?>
<root>
    <parse>
       <token>
           <reading furigana="がっこう">学校</reading>
       </token>
       <token>は</token>
       <token>
           <reading furigana="いえ">家</reading>
       </token>
       <token>から</token>
       <token>
           <reading furigana="とお">遠></reading>
           い
       </token>
       <token>の</token>
       <token>？</token>
    </parse>
</root>
```

#### Kana ####

URL: `/kana?str=*`

Example of rensponse for `/kana?str=学校は家から遠いの？`

```xml
<?xml version="1.0" encoding="UTF-8"?>
<root>
    <kana>がっこうはいえからとおいの？</kana>
</root>
```

### Compile it

    cmake . && make
