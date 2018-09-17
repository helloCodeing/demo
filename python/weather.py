#!/usr/local/bin/python3

import os
import re
import time
import requests
from datetime import datetime, timedelta
from bs4 import BeautifulSoup


headers = {
        'User-Agent': 'Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit'
                      '/537.36 (KHTML, like Gecko) Chrome/53.0.2785.143 Safar'
                      'i/537.36',
    }


def numtozh(num):
    num_dict = {1: '一', 2: '二', 3: '三', 4: '四', 5: '五', 6: '六', 7: '七',
                8: '八', 9: '九', 0: '零'}
    num = int(num)
    if 100 <= num < 1000:
        b_num = num // 100
        s_num = (num-b_num*100) // 10
        g_num = (num-b_num*100) % 10
        if g_num == 0 and s_num == 0:
            num = '%s百' % (num_dict[b_num])
        elif s_num == 0:
            num = '%s百%s%s' % (num_dict[b_num], num_dict.get(s_num, ''), num_dict.get(g_num, ''))
        elif g_num == 0:
            num = '%s百%s十' % (num_dict[b_num], num_dict.get(s_num, ''))
        else:
            num = '%s百%s十%s' % (num_dict[b_num], num_dict.get(s_num, ''), num_dict.get(g_num, ''))
    elif 10 <= num < 100:
        s_num = num // 10
        g_num = (num-s_num*10) % 10
        if g_num == 0:
            g_num = ''
        num = '%s十%s' % (num_dict[s_num], num_dict.get(g_num, ''))
    elif 0 <= num < 10:
        g_num = num
        num = '%s' % (num_dict[g_num])
    elif -10 < num < 0:
        g_num = -num
        num = '零下%s' % (num_dict[g_num])
    elif -100 < num <= -10:
        num = -num
        s_num = num // 10
        g_num = (num-s_num*10) % 10
        if g_num == 0:
            g_num = ''
        num = '零下%s十%s' % (num_dict[s_num], num_dict.get(g_num, ''))
    return num

def show_weather(day,wea,strong,today):
    space = "\t"
    day = day.strip()
    wea = wea.strip()
    strong = strong.strip()
    str = ""
    if today == day:
        str = day + space + wea + space + "温度: " + strong + space + "今日"
    else:
        str = day + space + wea + space + "温度: " + strong
    print('*'*60)
    print(str)

def day_weather(soup):
    #print(soup)
    weather = []
    day_info = soup.find('em').getText()
    #print(day_info)

    wea_info = soup.find('img')#x.tag['alt']
    wea_info = wea_info.attrs['alt']
    #print(wea_info)

    wea_list = soup.find_all('p')
    for wea in wea_list:
        num = wea.getText()
        weather.append(num)
    weather = (' ').join(weather)
    #print(weather)
    today = datetime.now().date().strftime('%d')
    show_weather(day_info,wea_info,weather,today)

def month_weather(soup):
    
    month = soup.find('div', attrs={'class': 'grid clearfix'})
    month_list = month.find_all('li',attrs={'class': 'item'})

    #print(len(month_list));

    for day in month_list:
        if day.find('em') == None:
            continue
        day_weather(day)
    

def get_weather():
    # 下载墨迹天气主页源码
    res = requests.get('http://tianqi.moji.com/', headers=headers)
    # 用BeautifulSoup获取所需信息
    soup = BeautifulSoup(res.text, "html.parser")
    month_weather(soup)
    return
    temp = soup.find('div', attrs={'class': 'wea_weather clearfix'}).em.getText()
    temp = numtozh(int(temp))
    weather = soup.find('div', attrs={'class': 'wea_weather clearfix'}).b.getText()
    sd = soup.find('div', attrs={'class': 'wea_about clearfix'}).span.getText()
    sd_num = re.search(r'\d+', sd).group()
    sd_num_zh = numtozh(int(sd_num))
    sd = sd.replace(sd_num, sd_num_zh)
    wind = soup.find('div', attrs={'class': 'wea_about clearfix'}).em.getText()
    qiche = soup.find('div', attrs={'class': 'wea_about clearfix'}).b.getText()
    aqi = soup.find('div', attrs={'class': 'wea_alert clearfix'}).em.getText()
    aqi_num = re.search(r'\d+', aqi).group()
    aqi_num_zh = numtozh(int(aqi_num))
    aqi = aqi.replace(aqi_num, aqi_num_zh).replace(' ', ',空气质量,')
    info = soup.find('div', attrs={'class': 'wea_tips clearfix'}).em.getText()
    sd = sd.replace(' ', '百分之').replace('%', '')
    aqi = '空气污染指数' + aqi
    info = info.replace('，', ',')

    local_info = soup.find('div', attrs={'class': 'search_default'}).em.getText()
    local_info = local_info.split('，')[0]
    print(local_info)
    # 获取今天的日期
    today = datetime.now().date().strftime('%Y年%m月%d日')
    # 将获取的信息拼接成一句话
    text = '早上好！今天是%s,%s今日%s,天气,%s,温度%s摄氏度,%s,%s,%s,%s' % \
           (today,local_info,qiche, weather, temp, sd, wind, aqi, info)
    return text


def text2voice(text):
    url = 'http://tts.baidu.com/text2audio?idx=1&tex={0}&cuid=baidu_speech_' \
          'demo&cod=2&lan=zh&ctp=1&pdt=1&spd=3&per=4&vol=5&pit=3'.format(text)
    # 直接播放语音
    os.system('mplayer "%s"' % url)


def main():
    # 获取需要转换语音的文字
    get_weather()
    #text = get_weather()
    # print(text)
    # text2voice(text)

if __name__ == '__main__':
    main()
