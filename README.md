# Texture-Synthesis
## 基本原理
使用一張原始圖檔進行水平及垂直拼貼成一張更大張的合成影像，其接縫部分使用動態規劃法之生產線排程問題，使用歐式距離計算來做為各像素之權重。

**歐式距離計算: D = √((R1−R2)^2+(G1−G2)^2+(B1−B2)^2)**



## 程式執行方式允許參數設定
```
main.exe –i infile –o outfile –r 50 –s 1024
```
- -i 輸入影像名稱
- -o 輸出影像名稱
- -r 重疊區域像素個數
- -s 紋理合成影像大小 (1024 x 1024)

## 範例

- 輸入檔

![rock2](https://user-images.githubusercontent.com/24976415/26863031-67fc43ec-4b82-11e7-851d-f2bae780969c.jpg)

- 輸出檔

![rock2_out](https://user-images.githubusercontent.com/24976415/26863033-6a590a94-4b82-11e7-8fe7-a2c90686bb3b.jpg)

