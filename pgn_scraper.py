import requests
from bs4 import BeautifulSoup as bs
import urllib2


_URL = 'http://www.pgnmentor.com' 

# functional
r = requests.get(_URL+"/files.html")
soup = bs(r.text)
urls = []
names = []
for i, link in enumerate(soup.findAll('a')):
    href = link.get('href')
    if href is None or not href.split("/")[0] == "players":
    	continue 
    _FULLURL = _URL + "/" + href
    print href
    if _FULLURL.endswith('.zip'):
        urls.append(_FULLURL)
        names.append(soup.select('a')[i].attrs['href'])

names_urls = zip(names, urls)
print "Now processing urls."
for name, url in names_urls:
    print url
    rq = urllib2.Request(url)
    res = urllib2.urlopen(rq)
    zipfile = open(name, 'wb')
    zipfile.write(res.read())
    zipfile.close()


