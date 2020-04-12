"start file type detect
"filetype on
"according file type load corresponding plugin
"filetype plugin on

set number
"自适应不同语言智能缩进
filetype indent on
"设置编辑时制表符占用空格数
set tabstop=4
"将制表符扩展为空格
set expandtab
"设置格式化时制表符占用空格数
set shiftwidth=4
"让vim把连续数量的空格视为一个制表符
set softtabstop=4
"设置缩进线
"set list lcs=tab:\|\ "(here is a space)
set foldmethod=syntax
set nofoldenable

syntax enable
syntax on
set term=xterm
"colorscheme darkblue
"colorscheme desert
colorscheme molokai
set bg=dark
set cursorline
"highlight Cursorline   cterm=NONE ctermbg=black ctermfg=green guibg=NONE guifg=NONE
set cursorcolumn
"highlight Cursorcolumn cterm=NONE ctermbg=black ctermfg=green guibg=NONE guifg=NONE
set laststatus=2
"set search high light
set hlsearch 
"ignore casr sensitive
set ignorecase
set t_Co=256      "在windows中用xshell连接打开vim可以显示色彩

"./tags;表示在文件的所在目录下查找名字为.tags的符号文件,后面一个分号代表查找不到的话向上递归到父目录，
"而我们只要在项目根目录放一个.tags即可
"set autochdir
"set tags=tags; 这两句产生的作用相同
"还需要做一个自动生成tags的插件
set tags=./.tags;,.tags

"define Leader as ;
let mapleader=";"

"define shortcut key to quit, write/quit, no-save/quit
nmap <Leader>q :q<CR>
nmap <Leader>wq :w<CR>:q<CR>
nmap <Leader>Q :q!<CR>


"gtags
set nocscopeverbose	"avoid Added cscope database /home/zhang.haiwen/cnos-code/test/CNOS-code-tsingma/GTAGS Press ENTER or type command to continue 
set cscopetag		"use cscope as tags command
set cscopeprg='gtags-cscope'	"use gtags-cscope replace cscope
let GtagsCscope_Auto_Load = 1
let GtagsCscope_Auto_Map = 1
let GtagsCscope_Quiet = 1
let gtags_file=findfile("GTAGS", ";")		"find gtags file
if !empty(gtags_file)
	exe "cs add" gtags_file
endif

"different vim copy/paste
let g:copy_file=$HOME . "/.vim_copybuffer"
function Write_copy_file()
call delete(g:copy_file)
let lines=split(@", "\n")
call writefile(lines,g:copy_file)
endfunction
 
function Read_copy_file()
let l:buf=readfile(g:copy_file)
let @"=join(l:buf,"\n")
normal ""p 
endfunction

map <silent> ;y :call Write_copy_file()<Enter>
map <silent> ;p :call Read_copy_file()<Enter>         

"nerdtree configuration
map <silent> nt :NERDTreeMirror<CR>
map <silent> nt :NERDTreeToggle<CR>
let NERDTreeWinPos="right"
let NERDTreeShowBookmarks=1
"autocmd VimEnter * NERDTree

"tagbar
nmap <silent> tb :TagbarToggle<CR>  
let g:tagbar_left=1             "default on right
let g:tagbar_width=30           "default width is 40
let g:tagbar_autofocus=1        "cursor on tagbar when tagbar start
let g:tagbar_sort=0             "default is sort

"switch from c file and h file
nmap <silent> <Leader>sw :FSHere<CR>

"vim-plug management 
call plug#begin('~/.vim/plugged')
Plug 'scrooloose/nerdtree', { 'on': 'NERDTreeToggle' }
Plug 'tomasr/molokai'
"support laststatus
Plug 'liuchengxu/eleline.vim'
"switch from c file and h file
Plug 'derekwyatt/vim-fswitch'
Plug 'majutsushi/tagbar'
call plug#end()



"代码动态检查
