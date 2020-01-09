１．g++ source.cpp -O3 -o main
でコンパイル
２．python3 plot.py main GA
でNrate=5~40まで５刻みで実行（論文での一つ目のケース）し、最後に結果をまとめて表示

その他いろいろな定数はrun.py内の変数を変更して設定してください。単位はすべて長さはm、時間はsecondです。（出力は論文にそろえてkm/hになってます）
