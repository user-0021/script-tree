# ScriptTree

- [ScriptTree](#scripttree)
- [scriptTreeとは](#scripttreeとは)
- [インストール方法](#インストール方法)
- [実行方法](#実行方法)
- [使い方](#使い方)
  - [help　コマンドの表示](#helpコマンドの表示)
  - [quit　システムの終了](#quitシステムの終了)
  - [save　Nodeの保存](#savenodeの保存)
  - [load　保存したファイルの読み込み](#load保存したファイルの読み込み)
  - [run　Nodeの起動](#runnodeの起動)
  - [connect　パイプの接続](#connectパイプの接続)
  - [disconnect　パイプの切断](#disconnectパイプの切断)
  - [list　Nodeリストの表示](#listnodeリストの表示)
  - [clear　コンソールバッファのクリア](#clearコンソールバッファのクリア)
  - [const set　コンストパイプの設定](#const-setコンストパイプの設定)
  - [timer run　WakeUpタイマーの起動](#timer-runwakeupタイマーの起動)
  - [timer stop　WakeUpタイマーの停止](#timer-stopwakeupタイマーの停止)
  - [timer set　WakeUpタイマーのピリオドの設定](#timer-setwakeupタイマーのピリオドの設定)
  - [timer get　WakeUpタイマーのピリオドの取得](#timer-getwakeupタイマーのピリオドの取得)

# scriptTreeとは

ROSのようにロボットの制御システムの構築を助けるツールです。  
C言語で作成可能なNode、matlabのようなNode管理システムを提供します。

# インストール方法

まず、依存関係を以下のコマンドでダウンロードします。

```
sudo apt install -y libreadline-dev
```

その後、以下のコマンドでカレントディレクトリにリポジトリをクローンします

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

# 実行方法

lunchオプションでNode管理システムを起動します。

```
./scriptTree lunch
```

起動に成功するとカレントディレクトリにLogフォルダが生成され、  
scriptTreeターミナルが起動します。quitで終了できます。

```
>>> quit
```

# 使い方

ScriptTreeを使うにはNodeプログラムを用意する必要があります。  
かんたんな数学Nodeであれば[mathNodes](https://github.com/user-0021/mathNodes)を使用することができます。  
自作のNodeを作成したい場合は[nodeSystem](https://github.com/user-0021/nodeSystem)
を使用して作成することができます。  
使用方法の例は[nodeExample](https://github.com/user-0021/nodeExample)を参照してください。

## help　コマンドの表示

`help`コマンドを使用して各種コマンド及び使い方の表示ができます。

```
>>> help
```

## quit　システムの終了

`quit`コマンドを使用してNodeシステムを終了することができます。

```
>>> quit
```

## save　Nodeの保存

`save`コマンドを使用して現在Activeなノードをファイルに保存できます。  
コンスト値なども保存されますがNodeシステム固有の値は保存されません。(Periodなど)

```
>>> save SAVE_FILE_PATH
```

## load　保存したファイルの読み込み

`load`コマンドを使用してsaveコマンドで保存したファイルからNodeを読み込みます。

```
>>> load LOAD_FILE_PATH
```

## run　Nodeの起動

`run`コマンドを使用して、Nodeプログラムをシステム上で起動できます。

```
>>> run NODE_FILE_PATH
```

また、-nameオプションでNodeシステムに登録する名前を変更できます。

```
>>> run NODE_FILE_PATH -name NODE_REGIST_NAME
```

## connect　パイプの接続

`connect`コマンドを使用して、NodeのInputパイプとOutputパイプを接続できます。  
なお、pipeLengthとpipeUnitが一致する必要があります。  
また、引数は入力パイプから指定する必要があります。

```
>>> connecr IN_NODE IN_PIPE OUT_NODE OUT_PIPE
```

## disconnect　パイプの切断

`disconnect`コマンドを使用して接続済みパイプを切断することができます。  
切断するには入力パイプ側を指定する必要があります

```
>>> disconnect IN_NODE IN_PIPE
```

## list　Nodeリストの表示

`list`コマンドを使用して現在ActiveなNodeのリスト表示できます

```
>>> list
```

## clear　コンソールバッファのクリア

`clear`コマンドを使用してコンソールバッファをクリアできます。  
shellのclearと等価です。

```
>>> clear
```

## const set　コンストパイプの設定

`const set`コマンドを使用してコンストパイプの値を設定できます。  
引数に渡す値はpipeLengthとpipeUnitを合わせる必要があります。

```
const set CONST_NODE CONST_PIPE SET_VALUE
```

pipeLengthがN>1の場合は次のように設定します。

```
cosnt set CONST_NODE CONST_IN SET_VALUE_1 ... SET_VALUE_N
```

## timer run　WakeUpタイマーの起動

`timer run`コマンドを使用してwakeUpタイマーを起動できます。  
wakeUpタイマーとはWait状態のNodeを定期的に起動するタイマーです。  
主に数学Nodeなどで、一定間隔での処理を行いたいときなどに使用されます。

```
>>> timer run
```

## timer stop　WakeUpタイマーの停止

`timer stop`コマンドを使用してwakeUpタイマーを停止できます。

```
>>> timer stop
```

## timer set　WakeUpタイマーのピリオドの設定

`timer set`コマンドを使用してwakeUpタイマーのピリオドを変更できます。  
指定する値の単位はmsです。  
なお、過渡解析などの小さな値のステップ時間が求められる場合では、0.01などの
十分小さい値を指定する必要があります。初期値は1000msです。

```
>>> timer set PERIOD_MS
```

## timer get　WakeUpタイマーのピリオドの取得

`timer get`コマンドを使用して現在のwakeUpタイマーのピリオドを取得できます。  

```
>>> timer get
```