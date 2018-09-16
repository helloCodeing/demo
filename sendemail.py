from smtplib import SMTP_SSL
from email.mime.text import MIMEText

with SMTP_SSL(host="smtp.qq.com") as smtp :
    smtp.login(user='654613310@qq.com',password='654613310long?')
    
    msg = MIMEText("这是来自Python3的一封测试邮件",_charset="utf8")
    msg["Subject"] = "测试邮件"
    msg["from"] = '654613310@qq.com'
    msg["to"] = '654613310@qq.com'
    
    smtp.sendmail(from_addr="654613310@qq.com",to_addrs="654613310@qq.com", msg=msg.as_string())
