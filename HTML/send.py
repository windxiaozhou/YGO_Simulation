import requests
from requests.structures import CaseInsensitiveDict
# 新加坡rest-host：rest-singapore.goeasy.io
url = "https://rest-hz.goeasy.io/v2/pubsub/publish"
headers = CaseInsensitiveDict()
headers["Accept"] = "application/json"
headers["Content-Type"] = "application/json"
data = """
{
  "appkey": "BC-af75f53b36a8449498ab18fc268d4d41",
  "channel": "test_channel",
  "content": "Hello, GoEasy!"
}
"""
resp = requests.post(url, headers=headers, data=data)
print(resp.status_code)