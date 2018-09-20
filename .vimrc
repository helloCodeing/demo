
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
" 系统环境设置
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
let g:ismac = 0
if(has("mac"))
    let g:ismac = 1
endif

"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
" 通用设置
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
set clipboard=unnamed         " 共享系统剪切板
let mapleader = ","           " 定义<leader>键
set nocompatible              " 设置不兼容原始vi模式
filetype on                   " 设置开启文件类型侦测
filetype plugin on            " 设置加载对应文件类型的插件
set noeb                      " 关闭错误的提示
syntax enable                 " 开启语法高亮功能
syntax on                     " 自动语法高亮
set t_Co=256                  " 开启256色支持
set cmdheight=2               " 设置命令行的高度
set showcmd                   " select模式下显示选中的行数
set ruler                     " 总是显示光标位置
set laststatus=2              " 总是显示状态栏
set number                    " 开启行号显示
set cursorcolumn              " 高亮列
set cursorline                " 高亮显示当前行
set whichwrap+=<,>,h,l        " 设置光标键跨行
set ttimeoutlen=0             " 设置<ESC>键响应时间
set virtualedit=block,onemore " 允许光标出现在最后一个字符的后面

"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
" 代码缩进和排版
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
set autoindent              " 设置自动缩进
set cindent                 " 设置使用C/C++语言的自动缩进方式
set cinoptions=g0,:0,N-s,(0 " 设置C/C++语言的具体缩进方式
set smartindent             " 智能的选择对其方式
filetype indent on          " 自适应不同语言的智能缩进
set expandtab               " 将制表符扩展为空格
set tabstop=4               " 设置编辑时制表符占用空格数
set shiftwidth=4            " 设置格式化时制表符占用空格数
set softtabstop=4           " 设置4个空格为制表符
set smarttab                " 在行和段开始处使用制表符
set nowrap                  " 禁止折行
set backspace=2             " 使用回车键正常处理indent,eol,start等
set sidescroll=10           " 设置向右滚动字符数
"set fdm=indent              " 设置代码折叠

"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
" 代码补全
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
set wildmenu             " vim自身命名行模式智能补全
set completeopt-=preview " 补全时不显示窗口，只显示补全列表

"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
" 搜索设置
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
set hlsearch            " 高亮显示搜索结果
set incsearch           " 开启实时搜索功能
set ignorecase          " 搜索时大小写不敏感

"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
" 缓存设置
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
set nobackup            " 设置不备份
set noswapfile          " 禁止生成临时文件
set autoread            " 文件在vim之外修改过，自动重新读入
set autowrite           " 设置自动保存
set confirm             " 在处理未保存或只读文件的时候，弹出确认

"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
" 编码设置
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
set langmenu=zh_CN.UTF-8
set helplang=cn
set termencoding=utf-8
set encoding=utf8
set fileencodings=utf8,ucs-bom,gbk,cp936,gb2312,gb18030

"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
" buffer 设置
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
nnoremap <leader>1 :1b<CR>
nnoremap <leader>2 :2b<CR>
nnoremap <leader>3 :3b<CR>
nnoremap <leader>4 :4b<CR>
nnoremap <leader>5 :5b<CR>
nnoremap <leader>6 :6b<CR>
nnoremap <leader>7 :7b<CR>
nnoremap <leader>8 :8b<CR>
nnoremap <leader>9 :9b<CR>
nnoremap <leader>0 :10b<CR>

"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
" 插件列表
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
call plug#begin('~/.vim/plugged')
Plug 'skywind3000/vim-preview'
Plug 'kana/vim-textobj-user'
Plug 'kana/vim-textobj-indent'
Plug 'kana/vim-textobj-syntax'
Plug 'kana/vim-textobj-function', { 'for':['c', 'cpp', 'vim', 'go','py'] }
Plug 'sgur/vim-textobj-parameter'
"Plug 'jsfaint/gen_tags.vim'
Plug 't9md/vim-choosewin'
Plug 't9md/vim-textmanip'
Plug 'mhinz/vim-signify'                " git svn diff
Plug 'mhinz/vim-startify'               " 启动页面
Plug 'roman/golden-ratio'               " 分屏自动适应
Plug 'terryma/vim-expand-region'        " 区块选中 + -
Plug 'chxuan/prepare-code'              " 自动
Plug 'chxuan/tagbar'                    " 函数列表
Plug 'Valloric/YouCompleteMe'           " 自动补全
Plug 'Yggdroot/LeaderF', { 'do': './install.sh' } " 神器，函数，文件，搜索
Plug 'easymotion/vim-easymotion'        " 快速移动
Plug 'jiangmiao/auto-pairs'             " 括号补全
Plug 'scrooloose/nerdtree', { 'on':'NERDTreeToggle'}              " 文件列表
Plug 'tpope/vim-repeat'                 
Plug 'tpope/vim-endwise'                " endif补全
Plug 'octol/vim-cpp-enhanced-highlight' " C++高亮
Plug 'vim-airline/vim-airline'          " 状态栏？
Plug 'vim-airline/vim-airline-themes'   
Plug 'Shougo/echodoc.vim'               " 函数提示
Plug 'rhysd/clever-f.vim'               " F键增强
Plug 'yianwillis/vimcdoc'               " vim中文文档:help查看
Plug 'fatih/vim-go'
Plug 'SirVer/ultisnips'
Plug 'honza/vim-snippets'
Plug 'scrooloose/nerdcommenter'         " 快速注释
Plug 'Yggdroot/indentLine'              " 游标线
Plug 'tpope/vim-surround'               " 成双成对编辑
Plug 'terryma/vim-multiple-cursors'     " 多行编辑
Plug 'lfv89/vim-interestingwords'       " 单词标记
Plug 'junegunn/vim-easy-align'          " 快速对齐
Plug 'luochen1990/rainbow'              " 多彩括号
Plug 'mileszs/ack.vim'                  " 快速查找
Plug 'rking/ag.vim'                     " 同ack
Plug 'iamcco/markdown-preview.vim'      " 写作预览
Plug 'kshenoy/vim-signature'            " 标签显示
Plug 'mbbill/undotree'                  " 后悔药
call plug#end()            

xmap <Space>d <Plug>(textmanip-duplicate-down)
nmap <Space>d <Plug>(textmanip-duplicate-down)
xmap <Space>D <Plug>(textmanip-duplicate-up)
nmap <Space>D <Plug>(textmanip-duplicate-up)

xmap <C-j> <Plug>(textmanip-move-down)
xmap <C-k> <Plug>(textmanip-move-up)
xmap <C-h> <Plug>(textmanip-move-left)
xmap <C-l> <Plug>(textmanip-move-right)

" toggle insert/replace with <F10>
nmap <F10> <Plug>(textmanip-toggle-mode)
xmap <F10> <Plug>(textmanip-toggle-mode)

"ack
"调用ag进行搜索
if executable('ag')
    let g:ackprg = 'ag --nogroup --nocolor --column'
endif
"nmap <leader>ck :Ack!<space>-i<space>
command! -nargs=* Search call InputAwareAckSearch(<q-args>)
nnoremap <Leader>/ :Search<space>
function! InputAwareAckSearch(args)
    if len(a:args) > 0
        " Executes ack by wrapping user input with quotes
        execute ":Ack! '".a:args."'"
    else
        " For no input, executes empty ack, so that word under cursor is searched
        execute ":Ack!"
    endif
endfunction
let g:ackhighlight = 1
let g:ack_qhandler = "botright copen 15"
let g:ack_autoclose = 1
let g:ack_use_cword_for_empty_search = 1

" load vim default plugin
runtime macros/matchit.vim



" 分屏窗口移动
nnoremap <c-j> <c-w>j
nnoremap <c-k> <c-w>k
nnoremap <c-h> <c-w>h
nnoremap <c-l> <c-w>l

" 打开文件自动定位到最后编辑的位置
autocmd BufReadPost * if line("'\"") > 1 && line("'\"") <= line("$") | execute "normal! g'\"" | endif


" airline
let g:airline_theme="tomorrow"
let g:airline_powerline_fonts = 1
let g:airline#extensions#tabline#enabled = 1
let g:airline#extensions#tabline#formatter = 'unique_tail'
let g:airline_section_b = '%{strftime("%m/%d/%Y - %H:%M")}'
if !exists('g:airline_symbols')
    let g:airline_symbols = {}
endif
let g:airline_left_sep = ' '
let g:airline_left_alt_sep = '|'
let g:airline_right_sep = ' '
let g:airline_right_alt_sep = ' '


" nerdtree
nnoremap <silent> <leader>n :NERDTreeToggle<cr>
let g:NERDTreeFileExtensionHighlightFullName = 1
let g:NERDTreeExactMatchHighlightFullName = 1
let g:NERDTreePatternMatchHighlightFullName = 1
let g:NERDTreeHighlightFolders = 1         
let g:NERDTreeHighlightFoldersFullName = 1 
let g:NERDTreeDirArrowExpandable='▷'
let g:NERDTreeDirArrowCollapsible='▼'

" YCM
let g:ycm_confirm_extra_conf = 0 
let g:ycm_error_symbol = '✗'
let g:ycm_warning_symbol = '✗'
let g:ycm_seed_identifiers_with_syntax = 1 
let g:ycm_complete_in_comments = 1 
let g:ycm_complete_in_strings = 1 

let g:ycm_server_python_interpreter = '/usr/bin/python'

let g:ycm_python_binary_path = 'python'
let g:ycm_global_ycm_extra_conf = '~/.ycm_confirm_extra_conf'

" ctags
set tags+=/usr/include/tags
set tags+=~/.vim/systags
set tags+=~/.vim/x86_64-linux-gnu-systags
let g:ycm_collect_identifiers_from_tags_files = 1
let g:ycm_semantic_triggers =  {
            \   'c' : ['->', '.','re![_a-zA-z0-9]'],
            \   'objc' : ['->', '.', 're!\[[_a-zA-Z]+\w*\s', 're!^\s*[^\W\d]\w*\s',
            \             're!\[.*\]\s'],
            \   'ocaml' : ['.', '#'],
            \   'cpp,objcpp' : ['->', '.', '::','re![_a-zA-Z0-9]'],
            \   'perl' : ['->'],
            \   'php' : ['->', '::'],
            \   'cs,java,javascript,typescript,d,python,perl6,scala,vb,elixir,go' : ['.'],
            \   'ruby' : ['.', '::'],
            \   'lua' : ['.', ':'],
            \   'erlang' : [':'],
            \ }
let g:ycm_semantic_triggers.c = ['->', '.', ' ', '(', '[', '&',']']

" tagbar
let g:tagbar_width = 30
nnoremap <silent> <leader>t :TagbarToggle<cr>
nnoremap <silent> <leader>rn :set rnu<cr>


" vim-easymotion
let g:EasyMotion_smartcase = 1
map <leader>o <Plug>(easymotion-bd-w)
nmap <leader>o <Plug>(easymotion-overwin-w)

" pydiction
let g:pydiction_location='~/.vim/plugged/pydiction/complete-dict'
let g:pydiction_menu_height=10

" nerdtree-git-plugin
let g:NERDTreeIndicatorMapCustom = {
            \ "Modified"  : "✹",
            \ "Staged"    : "✚",
            \ "Untracked" : "✭",
            \ "Renamed"   : "➜",
            \ "Unmerged"  : "═",
            \ "Deleted"   : "✖",
            \ "Dirty"     : "✗",
            \ "Clean"     : "✔︎",
            \ 'Ignored'   : '☒',
            \ "Unknown"   : "?"
            \ }

" LeaderF
nnoremap <leader>ft :LeaderfTag<cr>
nnoremap <leader>fl :LeaderfLineAll<cr>

let g:Lf_WildIgnore = {
            \ 'dir': ['.svn','.git','.hg','.vscode','.wine','.deepinwine','.oh-my-zsh'],
            \ 'file': ['*.sw?','~$*','*.bak','*.exe','*.o','*.so','*.py[co]']
            \}
let g:Lf_UseCache = 0

" echodoc.vim
let g:echodoc_enable_at_startup = 1

"set UltiSnips
let g:UltiSnipsExpandTrigger="<leader><tab>"
let g:UltiSnipsJumpForwardTrigger="<leader><tab>"
let g:UltiSnipsJumpBackwardTrgger="<leader><tab>"
let g:UltiSnipsListSnippets="<c-e>"

"LINX

" 使用 ALT+E 来选择窗口
nmap <leader>as <Plug>(choosewin)

" 默认不显示 startify
"let g:startify_disable_at_vimenter = 1
let g:startify_session_dir = '~/.vim/session'

" 使用 <space>ha 清除 errormarker 标注的错误
noremap <silent><space>ha :RemoveErrorMarkers<cr>

" signify 调优
let g:signify_vcs_list               = ['git', 'svn']
let g:signify_sign_add               = '+'
let g:signify_sign_delete            = '-'
let g:signify_sign_delete_first_line = '# '
let g:signify_sign_change            = '~'
let g:signify_sign_changedelete      = g:signify_sign_change

" git 仓库使用 histogram 算法进行 diff
let g:signify_vcs_cmds = {
        \ 'git': 'git diff --no-color --diff-algorithm=histogram --no-ext-diff -U0 -- %f',
        \}

" 主题
set background=dark
let g:onedark_termcolors=256
colorscheme monokai

nnoremap <leader>ud :UndotreeToggle<cr>
if has("persistent_undo")
    set undodir=~/.undodir/
    set undofile
endif

let g:prepare_code_plugin_path = expand($HOME . "/.vim/plugged/prepare-code")

inoremap jk <esc>
nnoremap <leader>w <C-W><C-K>
nnoremap <leader>s <C-W><C-J>
nnoremap <leader>a <C-W><C-h>
nnoremap <leader>d <C-W><C-l>
nnoremap <leader>v :sp<CR>
nnoremap <leader>h :vs<CR>
nnoremap <space><space>  %
nnoremap <leader>z zz
nnoremap <leader>sh :noh<cr>
nnoremap <leader>q :q!<CR>

nnoremap <leader>e :edit $MYVIMRC<cr> " 编辑vimrc文件
nnoremap <leader>H :execute ":help " . expand("<cword>")<cr>    


nmap <silent> <F8> <Plug>MarkdownPreview
imap <silent> <F8> <Plug>MarkdownPreview
nmap <silent> <F9> <Plug>StopMarkdownPreview
imap <silent> <F9> <Plug>StopMarkdownPreview

"LINX

let g:rainbow_active = 1
nnoremap zpr :setlocal foldexpr=(getline(v:lnum)=~@/)?0:(getline(v:lnum-1)=~@/)\\|\\|(getline(v:lnum+1)=~@/)?1:2 foldmethod=expr foldlevel=0 foldcolumn=2<CR>:set foldmethod=manual<CR><CR>

vmap <leader>aa <Plug>(EasyAlign)
nmap <leader>aa <Plug>(EasyAlign)

let g:easy_align_delimiters = {}
set cscopequickfix=s+,c+,d+,i+,t+,e+,a+
noremap  <leader>gt :cs find t <C-R>=expand('<cword>')<CR><CR>:copen<CR>
noremap  <leader>gs :cs find s <C-R>=expand('<cword>')<CR><CR>:copen<CR>
noremap  <leader>gi :cs find i <C-R>=expand('<cfile>')<CR><CR>:copen<CR>
noremap  <leader>gg :cs find g <C-R>=expand('<cword>')<CR><CR>:copen<CR>
noremap  <leader>gf :cs find f <C-R>=expand('<cfile>')<CR><CR>:copen<CR>
noremap  <leader>ge :cs find e <C-R>=expand('<cword>')<CR><CR>:copen<CR>
noremap  <leader>gd :cs find d <C-R>=expand('<cword>')<CR><CR>:copen<CR>
noremap  <leader>gc :cs find c <C-R>=expand('<cword>')<CR><CR>:copen<CR>



"自动载入ctags gtags
if version >= 800
    Plug 'ludovicchabant/vim-gutentags'
    Plug 'skywind3000/gutentags_plus'

    let $GTAGSLABEL = 'native-pygments'
    let $GTAGSCONF = '/usr/local/share/gtags/gtags.conf'

    " gutentags 搜索工程目录的标志，当前文件路径向上递归直到碰到这些文件/目录名
    let g:gutentags_project_root = ['.root', '.svn', '.git', '.hg', '.project']

    " 所生成的数据文件的名称
    let g:gutentags_ctags_tagfile = '.tags'

    " 同时开启 ctags 和 gtags 支持：
    let g:gutentags_modules = []
    if executable('ctags')
        let g:gutentags_modules += ['ctags']
    endif
    if executable('gtags-cscope') && executable('gtags')
        let g:gutentags_modules += ['gtags_cscope']
    endif

    " 将自动生成的 ctags/gtags 文件全部放入 ~/.cache/tags 目录中，避免污染工程目录
    let g:gutentags_cache_dir = expand('~/.cache/tags')

    " 配置 ctags 的参数
    let g:gutentags_ctags_extra_args = ['--fields=+niazS', '--extra=+q']
    let g:gutentags_ctags_extra_args += ['--c++-kinds=+px']
    let g:gutentags_ctags_extra_args += ['--c-kinds=+px']

    " 如果使用 universal ctags 需要增加下面一行
    let g:gutentags_ctags_extra_args += ['--output-format=e-ctags']

    " 禁用 gutentags 自动加载 gtags 数据库的行为
    " 避免多个项目数据库相互干扰
    " 使用plus插件解决问题
    let g:gutentags_auto_add_gtags_cscope = 0

    "预览 quickfix 窗口 ctrl-w z 关闭
    Plug 'skywind3000/vim-preview'
    "P 预览 大p关闭
    autocmd FileType qf nnoremap <silent><buffer> p :PreviewQuickfix<cr>
    autocmd FileType qf nnoremap <silent><buffer> P :PreviewClose<cr>
    "noremap <Leader>u :PreviewScroll -1<cr> " 往上滚动预览窗口
    "noremap <leader>d :PreviewScroll +1<cr> "  往下滚动预览窗口
endif


" ##########################自动命令设置##########################

" 启动vim后如果当前文件不存在则打开NERDTree
function! TryToOpenNERDTree()
    if !filereadable(glob("%"))
        NERDTree
    endif
endfunction
"au VimEnter * call TryToOpenNERDTree()
" 在c和cpp类型的文件中插入模式下按下-即为->, 方便指针调用
au Filetype c,cpp inoremap <buffer> - ->

" 启用TermDebug
au Filetype c,cpp packadd termdebug

" 自动保存和恢复一些vim的状态, 如光标位置, 折叠状态等待
au BufWinLeave * silent! mkview
au BufWinEnter * silent! loadview


if !exists('g:easy_align_delimiters')
endif
let g:easy_align_delimiters['#'] = { 'pattern': '#', 'ignore_groups': ['String'] }

let $GTAGSLABEL = 'native-pygments'
let $GTAGSCONF = '/usr/local/share/gtags/gtags.conf'

" 个性化
if filereadable(expand($HOME . '/.vimrc.local'))
    source $HOME/.vimrc.local
endif
func! RunResult()
    exec "w"
    if &filetype == "vim"
        exec "source %"
    else
        echo "not vim script"
    endif
endfunction

map <F5> :call RunResult() <CR>
imap <F5> <ESC>:call RunResult() <CR>
