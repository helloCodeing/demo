#! usr/bin/python3
#! -*- coding: utf-8 -*-
import requests
import time
from bs4 import BeautifulSoup

# url = "http://www.weather.com.cn/weather/101010100.shtml"
url = "http://www.weather.com.cn/weather/101210108.shtml"
Header ={"User-Agent":"Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.81 Safari/537.36"}

def get_weather():
	res = requests.get(url, headers = Header)
	soup = BeautifulSoup(res.content, 'html.parser')
	# 下面代码解释出当前的日期、温度、天气、风向、风力
	# 关键是，代码写死了，爬取网站代码一改，又要跟着调整。
	data = soup.find(class_="t clearfix")
	date = data.li.h1.text
	print("查询的日期：", date)
	wea = soup.find_all(class_="wea")[0].text.strip()
	print("天气概况：", wea)
	tem = soup.find_all(class_="tem")[0].text.strip()
	print("当前温度:",tem)
	win = soup.find_all(class_="win")[0].span['title'].strip()
	print("风向:",win)
	leve1 = soup.find_all(class_="win")[0].i.text.strip()
	print("风力:",leve1)


def severn_day():
    response = requests.get(url,headers=Header)
    response.encoding=response.apparent_encoding
    bs = BeautifulSoup(response.text,"html.parser")
    date = bs.select("li.sky > h1")
    desc = bs.select("li.sky > p.wea")
    temp = bs.select("li.sky > p.tem")
    dir = bs.select("li > p.win > em")
    level = bs.select("li > p.win > i")

    result = []
    for i in range(date.__len__()):
        date1 = date[i].text
        desc1 = desc[i].text
        temp1 = temp[i].stripped_strings
        temp1 = "".join(temp1)
        # print(temp1)
        dir1 = dir[i]
        dir2 = dir1.select("span")
        if len(dir2) == 1:
            direction = "无持续风向"
        else:
            # for c in dir2:
            direction = dir2[0].get("title")+"-"+dir2[1].get("title")
            # print(dir3)
        level1 = level[i].text
        result.append([date1,desc1,temp1,direction,level1])
    for result1 in result:
        print("\t".join(result1))


def main():
	# print("当前时间是：", time.asctime())
    get_weather()
    print('*'*60)
    severn_day()

if __name__ == "__main__":
    main()
