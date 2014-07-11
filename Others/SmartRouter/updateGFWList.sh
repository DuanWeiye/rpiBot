#!/bin/bash
# Convert AutoProxy rules to Privoxy forward rules.

echo == OpenWRT Update ==

svStatus=$(wget -qO- http://sv.brainexplode.com)
freeSpace=$(df | grep 'rootfs' | awk '{print $4}')
unSecureFile=/root/gfw_unsecure
actionFile=/root/gfw_online.action

if [ $freeSpace -gt 1024 ]; then
	echo + Free Space Check Passed.
else
	echo - No Enough Free Space.
	exit 1
fi	
			
if [ "$svStatus" == "OK" ]; then
        echo + Server ALive.

else
        echo - Server Down.
        exit 1
fi

ln -s /proc/self/fd /dev/fd
                      
rm -f $unSecureFile
wget http://sv.brainexplode.com/_gfwlist.txt -q -O- | base64 -d > $unSecureFile

if [ -f $unSecureFile ]; then
	echo + Unsecure GFW List Prepared.
else
        echo - Unable To Locate GFW List.
        exit 1
fi


# Spec: https://autoproxy.org/zh-CN/Rules
# Regular Expression                        # AutoProxy Rule Example            Privoxy Pattern
re_domain='^\|\|([^/]+)/?$'                 # ||example.com                     .example.com
re_start='^\|http(s?)://([^/\]+)([^\]*)'    # |https://ssl.example.com/foo*bar  ssl.example.com:443/foo.*bar
re_regex='^/(.*)/$'                         # /^https?:\/\/[^\/]+example\.com/  .*example.com
re_negate='^@@([^@].*)'                     # @@||example.com                   .example.com (NO PROXY)
re_comment='^!(.*)'                         # !Foo Bar                          #Foo Bar (comment)
re_keyword='^(https?://|/)?([^/|\][^/\]*)([^\]*)' # .example.com/foo*bar              .*.example.com:80/foo.*bar, :80/.*\.example\.com/foo.*bar (keyword filtering is HTTP only)

# Regular expression to shell expansion is lossy;
# this is the only regex used in practice, so far.
re_regex_a='^/\^https\?:\\/\\/\[\^\\/\]\+([^/]+)/$'

COMMENT=1
PROXY_TYPE=${PROXY_TYPE:-socks5}  # valid values are: http, socks4, socks4a, socks5
PROXY_ADDR=${PROXY_ADDR:-127.0.0.1:8124}  # forward to this address

PROXY_FOR=()
NOPROXY_FOR=()

atprxy2prvxy()
{
  local count=0
  local total=$(wc -l "$1" | awk '{print $1}') 
  local file="$1" line pattern
  while read -r line; do
    echo "= Reading line $count / $total"
    let "count=$count+1"
    _atprxy2prvxy "$line"
  done < <(sed '/^\[AutoProxy [0-9.]\+\]$/d' "${file}")
  if [ "${PROXY_TYPE^^}" == HTTP ]; then
    local forward_proxy="forward ${PROXY_ADDR}"
  else
    local forward_proxy="forward-${PROXY_TYPE,,} ${PROXY_ADDR} ." # assuming no parent HTTP proxy
  fi

  echo "+ Backup Old profile"
  mv -f "$actionFile" "$actionFile.bak"
  
  echo "{+forward-override{$forward_proxy}}" > $actionFile
  for pattern in "${PROXY_FOR[@]}"; do
    echo "$pattern" >> $actionFile
  done
  echo "{+forward-override{forward .}}" >> $actionFile
  for pattern in "${NOPROXY_FOR[@]}"; do
    echo "$pattern" >> $actionFile
  done

  echo "+ done"
}

# the main translater
_atprxy2prvxy()
{
  local s=$1 results=() noproxy=$2
  if [[ "$s" =~ $re_domain ]]; then
    results+=(".${BASH_REMATCH[1]}")
  elif [[ "$s" =~ $re_start ]]; then
    local https=${BASH_REMATCH[1]} port
    if [ "$https" ]; then
      port=443
    else
      port=80
    fi
    local one=${BASH_REMATCH[2]}
    local two=${BASH_REMATCH[3]}
    two=$(_sh2regex "$two") # must be in path
    if [ "${one:0:1}" == "*" ]; then
      results+=(".$one:$port$two") # one is domain
    else
      results+=("$one:$port$two") # one is domain
    fi
  elif [[ "$s" =~ $re_regex ]]; then
    if [[ "$s" =~ $re_regex_a ]]; then
      local one=$(_regex2sh "${BASH_REMATCH[1]}")
      results+=(".*$one")
    else # "I don't understand this regex!"
      local ans
      echo ":: Unkown regular expression found:" >&2
      echo "$s" >&2
      echo ":: Please convert the above to a privoxy pattern:" >&2
      read ans
      results+=("${ans}") # XXX may translate to more than one rule.
    fi
  elif [[ "$s" =~ $re_negate ]]; then
    _atprxy2prvxy "${BASH_REMATCH[1]}" 1
  elif [[ "$s" =~ $re_comment ]] && [ "$COMMENT" ]; then
    results+=("#${BASH_REMATCH[1]}")
  elif [[ "$s" =~ $re_keyword ]]; then
    local port=80 # keyword filtering is HTTP only
    local slash=${BASH_REMATCH[1]}
    local one=${BASH_REMATCH[2]}
    local two=${BASH_REMATCH[3]}
    if [ "${slash}" == / ]; then # both must in path
      one=$(_sh2regex "$one")
      two=$(_sh2regex "$two")
      results+=(":$port/(.*/)?$one$two")
    elif [ "${slash}" ]; then # keyword pattern with a scheme
      _atprxy2prvxy "|$s"
      results+=(":$port/.*$(_sh2regex "$s")")
    else
      local _one
      # It seems for privoxy's domain pattern, "*" does NOT match "."
      if [ "$two" ]; then
        _one=".*${one}"
      else
        _one=".*${one}*."
      fi
      two=$(_sh2regex "$two")
      results+=("${_one}:$port$two") # one in domain
      one=$(_sh2regex "$one")
      results+=(":$port/.*$one$two") # both in path
    fi
  elif [ -n "${s// /}" ]; then
    echo ":: Exception: unknown rule detected: '${s}'" >&2
  fi
  local r
  for r in "${results[@]}"; do
    _generate_privoxy_forward "$r" $noproxy
  done
}

# * ? [a-z] => regex
# seems only `*' is used?
# `?' seems to be treated as a literal.
_sh2regex()
{
  # Again, no backslash is used in practice.
  # Casual non-exhaustive escaping >.>
  sed 's/[{(^.?$)}]/\\&/g;s/\*/.&/g' <<< "$1"
}

# .* -> *  x? -> ?  . -> ?  \ ->
_regex2sh()
{
  # In practice, \\ won't be in the URL regex.
  # assuming really simple regex.
  sed 's/\.\*/\*/g;s/\([^\]\)\(?\)/?/g;s/\([^\]\)\(\.\)/\1?/g;s/\\//g' <<< "$1"
}

_generate_privoxy_forward()
{
  local pattern=$1 noproxy=$2
  if [ "${noproxy}" ]; then
    NOPROXY_FOR+=("${pattern}")
  else
    PROXY_FOR+=("${pattern}")
  fi
}

echo "+ Generating Privoxy Forward Rules" >&2
atprxy2prvxy "$unSecureFile"

/etc/init.d/privoxy restart

# vim: ft=sh sw=2:
