import sys
import requests
import json

# GET
search=sys.argv[1]
res = requests.get('https://api.dictionaryapi.dev/api/v2/entries/en/'+str(search))

if str(res.status_code)=="404":
    print("없는 단어입니다.")
else:
    val=json.loads(res.text)
    print(val[0]["meanings"][0]["definitions"][0]["definition"])
