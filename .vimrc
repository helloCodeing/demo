
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
set fdm=indent              " 设置代码折叠

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

"Plug 'skywind3000/vim-keysound'         " 打字机音效
Plug 'roman/golden-ratio'
Plug 'chxuan/tagbar'                    " 函数列表
Plug 'Valloric/YouCompleteMe'           " 自动补全
Plug 'Yggdroot/LeaderF'                 " 神器，函数，文件，搜索
Plug 'easymotion/vim-easymotion'        " 快速移动
Plug 'jiangmiao/auto-pairs'             " 括号补全
Plug 'scrooloose/nerdtree'              " 文件列表
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
Plug 'iamcco/markdown-preview.vim'
Plug 'kshenoy/vim-signature'            " 标签显示
Plug 'mbbill/undotree'

call plug#end()            

"ack
"调用ag进行搜索
if executable('ag')
    let g:ackprg = 'ag --nogroup --nocolor --column'
endif
nmap <leader>ck :Ack!<space>-i<space>
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
nnoremap <leader>u :YcmCompleter GoToDeclaration<cr>
nnoremap <leader>o :YcmCompleter GoToInclude<cr>
nnoremap <leader>ff :YcmCompleter FixIt<cr>
nmap <F5> :YcmDiags<cr>


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
nnoremap <leader>ff :LeaderfFunction<cr>
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
" 主题
set background=dark
let g:onedark_termcolors=256
colorscheme monokai

nnoremap <leader>ud :UndotreeToggle<cr>
if has("persistent_undo")
    set undodir=~/.undodir/
    set undofile
endif

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
nnoremap <leader>q :wq<CR>

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
if !exists('g:easy_align_delimiters')
    let g:easy_align_delimiters = {}
endif
let g:easy_align_delimiters['#'] = { 'pattern': '#', 'ignore_groups': ['String'] }

"let g:keysound_enable = 1      " 启动 Vim 时自动启动
"let g:keysound_theme = 'mario' " 设置默认音效主题，可以选择：default, typewriter, mario, bubble, sword
"let g:keysound_volume = 1000   " 设置音量：0-1000

" 个性化
if filereadable(expand($HOME . '/.vimrc.local'))
    source $HOME/.vimrc.local
endif

"func! RunResult()
    "exec "w"
    "if &filetype == "vim"
        "exec "source %"
    "else
        "echo "not vim script"
    "endif
"endfunction

"map <F5> :call RunResult() <CR>
"imap <F5> <ESC>:call RunResult() <CR>
