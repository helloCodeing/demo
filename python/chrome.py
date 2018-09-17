# from selenium import webdriver
# browser = webdriver.Chrome()

from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from bs4 import BeautifulSoup
# url ='http://www.2222zv.com/html/artdata/?id=33888'
# url_='http://www.2222zv.com/html/artlist/dushiyanqing/13.html'
url = 'https://image.baidu.com/'
def main():
    chrome_options = Options()
    chrome_options.add_argument('--headless')
    chrome_options.add_argument('--disable-gpu')
    driver_one = webdriver.Chrome(chrome_options=chrome_options)
    driver_one.implicitly_wait(10)

    driver_one.get(url)
    html = driver_one.page_source
    driver_one.close()
    soup = BeautifulSoup(html,'html.parser')
    print(soup)
    # print(driver_one.page_source)


if __name__ == '__main__':
    main()
