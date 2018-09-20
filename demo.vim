function! Foo()
python3 << EOF
class Foo_demo:
    def __init__(self):
        print ('Foo_demo init')
Foo_demo()
EOF
endfunction

function! Bar()
python3 << EOF
import vim
cur_buf = vim.current.buffer
print('line is {0}'.format(len(cur_buf)))
print('contents:{0}'.format(cur_buf[-3]))
EOF
endfunction


function! ShowLine()
python3 << EOF
import vim
line = vim.current.line
print(line)
EOF
endfunction


function! Append()
python3 << EOF
import vim
cur_buf = vim.current.buffer
cur_win = vim.current.window
row = cur_win.row
col = cur_win.col
print(row)
print(col)
lens = len(cur_buf)
print('current buf lens is ',lens)
# hello world
# hello world
cur_buf.append('# hello world',lens - 3)
print('current buf name is ',cur_buf.name)
# hello world
# hello world
print('current buf number is ',cur_buf.number)
EOF
endfunction


function! Print()
python3 << EOF
import vim
cur_buf = vim.current.buffer
for line in cur_buf:
    print(line)
EOF
endfunction


function! ChangeBuffer()
python3 << EOF
import vim
cur_buf_num = vim.options['eventignore']
print(cur_buf_num)
vim.options['eventignore'] = 'all'
try:
    vim.current.buffer = vim.buffers[1]
finally:
    vim.options['eventignore'] = cur_buf_num
EOF
endfunction

function! Mark()
python3 << EOF
import vim      
import re
def add_comment():
    cb = vim.current.buffer
    cw = vim.current.window
    row = cw.cursor[0]
    line_str = cb[row-1]

    const_str0 = '/**'
    const_str1 = ' * '
    const_str2 = ' */'
    str_list = []
    str_list.append(const_str0)

    largs = re.findall(r'\w+', line_str)
    if len(largs) <=1:
        print('you maybe at wrong line now' )
        return

    ret = largs[0]
    del largs[0]

    fun_name = largs[0]

    del largs[0]

    str_list.append(const_str1 + fun_name + ' - ')
    str_list.append(const_str1 + '@return:' + ' - ')
    for i,item in enumerate(largs):
        if i%2 == 1:
            str_list.append(const_str1 + '@' + item + ' - ')

    str_list.append(const_str2)
    cb[row-1:row-1] = str_list
    return

# hello world
# hello world
# hello world
add_comment()  
EOF
endfunction
