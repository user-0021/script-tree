# How to use linear list
**do_notが先頭についた関数の使用は非推奨です**

>**LINEAR_LIST_CREATE(type)**
>線形リストを作製するために使用されますtypeには作成したいリストの変数型を入れてください。
>返り値にはリストが返され、型は type* です。

\
\
\
>**LINEAR_LIST_NEXT(list)**
>listの次の要素を返します。次の要素がない場合返り値はNULLです。

\
\
\
>**LINEAR_LIST_PREV(list)**
>listの前の要素を返します。前の要素がない場合返り値はNULLです。

\
\
\
>**LINEAR_LIST_RELEASE(list)**
>線形リストを開放します。
>_線形リストを破棄するときに必ずこの関数を使用してください。_

\
\
\
>**LINEAR_LIST_PUSH(list,data)**
>線形リストにdataを追加します。
>追加された要素はリストの最も後ろへ追加されます。

\
\
\
>**LINEAR_LIST_LAST(list)**
>線形リストの最後の要素を取得します

\
\
\
>**LINEAR_LIST_FOREACH(list,iter){ do }**
>リストの各要素にdoを実行します。
>doの部分に好きなコードを追加してください
>iterはlistと同じ型にしてください

\
\
\
>**LINEAR_LIST_FOREACH_R(list,iter){ do }**
>LINEAR_LIST_FOREACHとほぼ一緒ですがこちらは後ろから順番に要素を取り出します。

\
\
\
