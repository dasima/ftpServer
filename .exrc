if &cp | set nocp | endif
let s:cpo_save=&cpo
set cpo&vim
inoremap <silent> <S-Tab> =BackwardsSnippet()
inoremap <silent> <expr> <Plug>delimitMateS-BS delimitMate#WithinEmptyPair() ? "\<Del>" : "\<S-BS>"
inoremap <silent> <Plug>delimitMateBS =delimitMate#BS()
snoremap <silent> 	 i<Right>=TriggerSnippet()
map  :NERDTreeToggle
nnoremap <silent>  :CtrlP
snoremap  b<BS>
snoremap % b<BS>%
snoremap ' b<BS>'
snoremap U b<BS>U
snoremap \ b<BS>\
snoremap ^ b<BS>^
snoremap ` b<BS>`
nmap gx <Plug>NetrwBrowseX
snoremap <Left> bi
snoremap <Right> a
snoremap <BS> b<BS>
snoremap <silent> <S-Tab> i<Right>=BackwardsSnippet()
nnoremap <silent> <Plug>NetrwBrowseX :call netrw#NetrwBrowseX(expand("<cWORD>"),0)
inoremap <silent> 	 =TriggerSnippet()
inoremap <silent> 	 =ShowAvailableSnips()
let &cpo=s:cpo_save
unlet s:cpo_save
set autoindent
set autoread
set backspace=2
set expandtab
set fileencodings=ucs-bom,utf-8,default,latin1
set fillchars=vert:|,fold:-,stl:\ ,stlnc:\\
set helplang=cn
set hlsearch
set printoptions=paper:letter
set ruler
set runtimepath=~/.vim/bundle/vundle,~/.vim/bundle/vim-rails,~/.vim/bundle/nerdtree,~/.vim/bundle/delimitMate,~/.vim/bundle/vim-markdown,~/.vim/bundle/ctrlp.vim,~/.vim/bundle/snipmate.vim,~/.vim,/var/lib/vim/addons,/usr/share/vim/vimfiles,/usr/share/vim/vim73,/usr/share/vim/vimfiles/after,/var/lib/vim/addons/after,~/.vim/after,~/.vim/bundle/vundle/,~/.vim/bundle/vundle/after,~/.vim/bundle/vim-rails/after,~/.vim/bundle/nerdtree/after,~/.vim/bundle/delimitMate/after,~/.vim/bundle/vim-markdown/after,~/.vim/bundle/ctrlp.vim/after,~/.vim/bundle/snipmate.vim/after
set shiftwidth=4
set showmatch
set smartindent
set suffixes=.bak,~,.swp,.o,.info,.aux,.log,.dvi,.bbl,.blg,.brf,.cb,.ind,.idx,.ilg,.inx,.out,.toc
set tabstop=4
set wildignore=*/tmp/*,*.so,*.swp,*.zip
set window=9
" vim: set ft=vim :
