import requests
from urllib.parse import urlencode
from bs4 import BeautifulSoup
headers = {
    'Host': 'www.2222zv.com',
    'User-Agent': 'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/69.0.3497.81 Safari/537.36',
    'X-Requested-With': 'XMLHttpRequest',
}

params = {
    "id": "19318991",
    "rt": "1536824720496",
    "rl": "2560*1440",
    "lang": "zh-CN",
    "ct": "unknow",
}

# home_url = 'https://www.2222zv.com/html/vodplay/46324.json?'
base_url = 'https://www.2222zv.com/html/vodlist/rh/5.html'
home_url = 'https://www.2222zv.com/html/vodplay/'
json_suffix = '.json?'

def get_html():
    try:
        url = base_url + urlencode(params)
        res = requests.get(url,headers=headers)
        res.encoding=res.apparent_encoding
        return BeautifulSoup(res.text,'html.parser')
    except requests.ConnectionError:
        return None

def movie_url(movie_id):
    if movie_id == "":
        return ""
    return home_url + movie_id + json_suffix

def splice_url(substr):
    href = substr.get('href')
    if href.find("id") == -1:
        return ""
    id_pos = href.find("=")
    movie_id = href[id_pos + 1:]
    return movie_id

def get_thunder(url):
    if url == "":
        return
    # url = base_url + urlencode(params)
    res = requests.get(url)

    print(res.text)
    json = res.json()
    print(json.get('down'))

def parser_page():
        soup = get_html()
        if soup == None:
            return
        hrefs = soup.find_all('a')
        for h in hrefs:
            movie_id = splice_url(h)
            m_url = movie_url(movie_id)
            get_thunder(m_url)


def main():
    parser_page()

if __name__ == "__main__":
   main()
