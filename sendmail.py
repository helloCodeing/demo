import subprocess

title = 'echo "天气预报" | mail  -s ' 
text = 
mail = '17633558906@163.com'
# pic = 'ls'
cmd = "%s" %text
result = subprocess.getoutput(cmd)
print(result)
