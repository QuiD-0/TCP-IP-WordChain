from sys import argv
from requests import get
from json import loads
data="";
# 구글 딕셔너리 API 사용 
search=argv[1]
res = get('https://api.dictionaryapi.dev/api/v2/entries/en/'+str(search))
if str(res.status_code)=="404":
    data="404"
else:
    val=loads(res.text)
    data=val[0]["meanings"][0]["definitions"][0]["definition"]
f = open("./api.txt", 'w')
f.write(data)
f.close()

