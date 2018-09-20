#!/usr/local/bin/python3

import os
import re
import time
import requests
import subprocess
from datetime import datetime, timedelta
from bs4 import BeautifulSoup

seven_url = 'https://tianqi.moji.com/forecast7/china/zhejiang/fuyang-district'

mail = '17633558906@163.com'

headers = {
        'User-Agent': 'Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit'
                      '/537.36 (KHTML, like Gecko) Chrome/53.0.2785.143 Safar'
                      'i/537.36',
    }

def get_html(url):
    res = requests.get(url, headers=headers)
    soup = BeautifulSoup(res.text, "html.parser")
    return soup

def get_day_info(html):
    day_info = []
    day = html.find_all('span',class_ = 'week')
    for info in day:
        day_ = info.getText()
        day_info.append(day_)

    day_info.reverse()
    day_info = (' ').join(day_info)
    return day_info

def show_weather(day,wea,b,strong):
    space = "\t"
    br = ''
    brr ='\n'
    str = br + day + space + wea + space + "最高温度: " + b + space + "最低温度: " + strong + brr
    print('*'*60)
    print(str)
    return str

def weather_info(html):
    day_info = get_day_info(html)
    wea = html.find('span',class_ = 'wea').getText()
    b = html.find('div',class_ = 'tree clearfix').b.getText()
    strong = html.find('div',class_ = 'tree clearfix').strong.getText()
    return show_weather(day_info,wea,b,strong)

def parse_html(url):
    soup = get_html(url)
    detail_future = soup.find('div',id = 'detail_future')
    lispan = detail_future.find_all('li')
    weather = ""
    for li in lispan:
        wea = weather_info(li)
        weather = weather + "\n"
        weather  = weather +  wea
    return weather

def sendmail(text):
    # pic = 'ls'
    # text = subprocess.getoutput('./python/mail.py')
    # pi = ' 天气预报 '
    # title = 'echo "%s \n  \n %s" | mail  -s ' %text %text
    # cmd = title + pi + mail
    # result = subprocess.getoutput(cmd)
    # print(result)
    # print(cmd)
    print(text)

def main():
    text = parse_html(seven_url)
    sendmail(text)

if __name__ == "__main__":
    main()
