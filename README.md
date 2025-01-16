# ScriptTree

## scriptTreeとは

ROSのようにロボットの制御システムの構築を助けるツールです。

C言語で作成可能なNode、matlabのようなNode管理システムを提供します。

## インストール方法

以下のコマンドでカレントディレクトリにリポジトリをクローンします

```
git clone https://github.com/user-0021/script-tree.git
```

その後クローンしたディレクトリへ移動しMakefileを実行します。

```
cd script-tree
make
```

scriptTreeが生成されていればインストール完了です。
実行すればバージョンが表示されるはずです。

```
./scriptTree
```

## 実行方法

lunchオプションでNode管理システムを起動します。

```
./scriptTree lunch
```

起動するとカレントディレクトリにLogフォルダが生成されます。

