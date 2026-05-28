#!/usr/bin/env bash
# Exercise the auto-generated REST endpoints. Run after starting:
#   ./build/auto_rest

set -euo pipefail

HOST=${HOST:-http://127.0.0.1:8080}

echo "-- create a Person --"
RESP=$(curl -s -X POST $HOST/Person)
echo "$RESP"
ID=$(echo "$RESP" | python3 -c "import sys, json; print(json.load(sys.stdin)['id'])")
echo "id=$ID"

echo
echo "-- write name --"
curl -s -X PUT $HOST/Person/$ID/name \
     -H 'Content-Type: application/json' \
     -d '"Alice"' -w "  (HTTP %{http_code})\n"

echo
echo "-- read name --"
curl -s $HOST/Person/$ID/name
echo

echo
echo "-- write age (valid) --"
curl -s -X PUT $HOST/Person/$ID/age \
     -H 'Content-Type: application/json' \
     -d '42' -w "  (HTTP %{http_code})\n"

echo
echo "-- read age --"
curl -s $HOST/Person/$ID/age
echo

echo
echo "-- range violation (age=999) --"
curl -s -X PUT $HOST/Person/$ID/age \
     -H 'Content-Type: application/json' \
     -d '999' -w "  (HTTP %{http_code})\n"
echo

echo
echo "-- readonly violation (id) --"
curl -s -X PUT $HOST/Person/$ID/id \
     -H 'Content-Type: application/json' \
     -d '"p-999"' -w "  (HTTP %{http_code})\n"
echo

echo
echo "-- call greet --"
curl -s -X POST $HOST/Person/$ID/greet \
     -H 'Content-Type: application/json' \
     -d '["Hello"]'
echo

echo
echo "-- delete --"
curl -s -X DELETE $HOST/Person/$ID -w "  (HTTP %{http_code})\n"

echo
echo "-- 404 after delete --"
curl -s $HOST/Person/$ID/name -w "  (HTTP %{http_code})\n"
echo
