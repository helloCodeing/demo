#!/usr/local/bin/python3

import os
import re
import time
import requests
from datetime import datetime, timedelta
from bs4 import BeautifulSoup

seven_url = 'https://tianqi.moji.com/forecast7/china/zhejiang/fuyang-district'

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
    str = day + space + wea + space + "最高温度: " + b + space + "最低温度: " + strong
    print('*'*60)
    print(str)

def weather_info(html):
    day_info = get_day_info(html)
    wea = html.find('span',class_ = 'wea').getText()
    b = html.find('div',class_ = 'tree clearfix').b.getText()
    strong = html.find('div',class_ = 'tree clearfix').strong.getText()
    show_weather(day_info,wea,b,strong)

def parse_html(url):
    soup = get_html(url)
    detail_future = soup.find('div',id = 'detail_future')
    lispan = detail_future.find_all('li')
    for li in lispan:
        weather_info(li)

    

def main():
    parse_html(seven_url)

if __name__ == "__main__":
    main()
