import sys
import requests
import json
data="";
# 구글 딕셔너리 API 사용 
search=sys.argv[1]
res = requests.get('https://api.dictionaryapi.dev/api/v2/entries/en/'+str(search))

if str(res.status_code)=="404":
    data="404"
else:
    val=json.loads(res.text)
    data=val[0]["meanings"][0]["definitions"][0]["definition"]
f = open("./api.txt", 'w')
f.write(data)
f.close()
