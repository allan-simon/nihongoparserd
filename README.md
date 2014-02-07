nihongoparserd
===========

A service to convert japanese languages into romaji, kana etc.

for the moment it's based on libmecab but it may be replaced by homebrew parser in the future

### Requirement ###

  * libexpat
  * libevent
  * libmecab2

### Usage ###

    ./nihonggoparserd -p PORT

That will launch an HTTP server listening on port PORT

it provides the following API call, that will return a XML answer

  * /kana?str=\*

### Compile it

    TODO
