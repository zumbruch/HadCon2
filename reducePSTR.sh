grep PSTR\(\" *.c -n | sed 's/\.c:/.c /g' | perl -p -e 's/(.*?) (\d+?):.*(PSTR\(\".*?\"\)).*$/\3 \1 \2/g' | sort > reducePSTR.txt
