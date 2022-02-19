#ifdef _WIN32 // sadece windows isletim sisteminde eklenecek kutuphaneler
#include <windows.h>
#endif
#ifndef _WIN32 // windows disindaki isletim sistemlerinde eklememiz gereken kutuphaneler
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define M 100
#define R 4             // rotation matrisinin boyutu RxR
#define DELAY 100       // milisaniye cinsinden uyuma suresi, 1s = 1000ms
#define CLEAR_DELAY 350 // satir silinirken milisaniye cinsinden uyuma suresi

void printImg(char img[M][M], int imgColors[M][M], int rowL, int columnL);
void resetImg(char img[M][M], int imgColors[M][M], int rowL, int columnL);
void imgSurround(char img[M][M], int imgColors[M][M], int rowL, int columnL);
int slctTetro(
    int rowL,
    int columnL,
    int *ptr_tetroColor,
    int *ptr_tetroH,
    int *ptr_tetroW,
    int *ptr_startC,
    int *ptr_skipR1,
    int *ptr_skipC1,
    int *ptr_skipR2,
    int *ptr_skipC2);
void printRotation(char rotation[R][R], int tetroColor, int rowL, int columnL);
void rotateTetroRight(char rotation[R][R]);
void rotateTetroLeft(char rotation[R][R]);
int addTetro(
    char img[M][M],
    int imgColors[M][M],
    int tetroColor,
    int rowL,
    int columnL,
    int tetroH,
    int tetroW,
    int startC,
    int skipR1,
    int skipC1,
    int skipR2,
    int skipC2);
int obstacleControl(
    char img[M][M],
    int rowL,
    int columnL,
    int tetroW,
    int tetroH,
    int startC,
    int botRow,
    int skipR1,
    int skipC1,
    int skipR2,
    int skipC2);
void tetroFall(
    char img[M][M],
    int imgColors[M][M],
    int rowL,
    int columnL,
    int tetroH,
    int tetroW,
    int startC,
    int botRow,
    int skipR1,
    int skipC1,
    int skipR2,
    int skipC2);
int clearRow(char img[M][M], int imgColors[M][M], int rowL, int columnL, int botRow, int tetroH);
void mySleep(int milliseconds);
void clearImgFromScreen(int rowL, int columnL);

// Konsolla etkilesim fonksiyonlari ve kodlari saklayan enum veri tipleri
enum TextColorCodes {
    RESET_COLOR,
    BLACK = 30,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE
};
enum ClearLineCodes {
    CLEAR_LINE_FROM_CURSOR_TO_END,
    CLEAR_LINE_FROM_CURSOR_TO_BEGIN,
    CLEAR_LINE_ALL
};
enum ClearScreenCodes {
    CLEAR_SCREEN_FROM_CURSOR_DOWN,
    CLEAR_SCREEN_FROM_CURSOR_UP,
    CLEAR_SCREEN_ALL
};

void setupConsole(); // windows icin konsolu ANSI Escape kodlarini kabul edecek sekilde ayarliyor
void resetConsole(); // konsolu varsayilan hale donduruyor
void setTextColor(int code);
void clearConsoleLine(int code);
void clearConsoleScreen(int code);
void moveCursorTo(int row, int column);
void moveCursorToBeginning();
void saveCursorPosition();
void restoreCursorPosition();
void hideCursor();
void showCursor();

int main()
{
    char playOrExit = 'p';
    int topScore = 0;

    setupConsole();
    srand(time(0));

    while(playOrExit != 'e' && playOrExit != 'E') {
        //MAIN MENU
        moveCursorToBeginning();
        printf("      ________  ________  ________  _______   ______   ______         \n"); //TETRIS sign
        printf("     |        \\|        \\|        \\|       \\ |      \\ /      \\  \n");
        printf("      \\$$$$$$$$| $$$$$$$$ \\$$$$$$$$| $$$$$$$\\ \\$$$$$$|  $$$$$$\\  \n");
        printf("        | $$   | $$__       | $$   | $$__| $$  | $$  | $$___\\$$      \n");
        printf("        | $$   | $$  \\      | $$   | $$    $$  | $$   \\$$    \\     \n");
        printf("        | $$   | $$$$$      | $$   | $$$$$$$\\  | $$   _\\$$$$$$\\    \n");
        printf("        | $$   | $$_____    | $$   | $$  | $$ _| $$_ |  \\__| $$      \n");
        printf("        | $$   | $$     \\   | $$   | $$  | $$|   $$ \\ \\$$    $$    \n");
        printf("         \\$$    \\$$$$$$$$    \\$$    \\$$   \\$$ \\$$$$$$  \\$$$$$$   ");
        setTextColor(YELLOW);
        moveCursorTo(10, 50);
        printf("by Erkan VATAN");
        setTextColor(RESET_COLOR);
        moveCursorTo(12, 28);
        printf("TOP SCORE: %d", topScore);
        moveCursorTo(14, 1);
        setTextColor(YELLOW);
        printf("                   INFORMATION ABOUT THE GAMEPLAY\n\n");
        setTextColor(RESET_COLOR);
        printf("1- The reference block to place the tetromino is the leftmost block.\n");
        printf("2- Any input higher than the playing field's number of columns will align tetromino to right wall.\n");
        printf("3- Columns that are multiples of five have red indicators to count easier (5th column, 10th column etc.).\n");
        printf("4- Suggested playing field size is 12x10. Any size lower than 4 will be replaced with 4.\n");
        printf("5- You can increase font size in terminal settings to see better.\n");
        setTextColor(RED);
        printf("\n\n              Enter any key to play, [E] to exit the game: ");
        setTextColor(RESET_COLOR);
        scanf(" %c", &playOrExit);

        if(playOrExit != 'e' && playOrExit != 'E') {
            //IN GAME SCREEN
            char img[M][M];      // tetris oyun ekranini temsil eden matris
            int imgColors[M][M]; // oyun ekranindaki her gozun renk degerini saklayan matris
            int rowL;            // oyun ekraninin satir sayisi
            int columnL;         // oyun ekraninin sutun sayisi

            int botRow;         // tetrominonun en altinin bulundugu satir
            int startC;         // tetrominonun en sol sutunu
            int tetroW;         // tetromino genisligi
            int tetroH;         // tetromino yuksekligi
            int tetroColor;     // tetrominonun hangi renkte oldugu
            int skipR1, skipC1; // tetrominoya dahil olmayan birinci satir-sutun kombinasyonu
            int skipR2, skipC2; // tetrominoya dahil olmayan ikinci satir-sutun kombinasyonu

            int continueGame = 1; // oyunun devam edip etmeyecegini belirleyen degisken
            int totalClear = 0;   // silinen toplam satir sayisi
            int clearCount;       // bir parca yerlestirmede silinen satir sayisi
            int score = 0;
            char scanfGarbage; // oyun ekraninin tusa basilana kadar durmasini saglamak icin scanf'te kullanacagimiz degisken

            clearConsoleScreen(CLEAR_SCREEN_ALL);
            moveCursorTo(5, 10);
            printf("Number of rows of the playing field   : ");
            scanf("%d", &rowL);
            if(rowL < 4)
                rowL = 4; // I parcasinin sigamayacagi bir deger girildiyse duzenliyoruz
            rowL += 2;    // ust ve altta bir satir kullanilmadigindan 2 ekliyoruz
            moveCursorTo(7, 10);
            printf("Number of columns of the playing field: ");
            scanf("%d", &columnL);
            if(columnL < 4) // I parcasinin sigamayacagi bir deger girildiyse duzenliyoruz
                columnL = 4;
            columnL += 2; // sag ve solda bir sutun kullanilmadigindan 2 ekliyoruz

            clearConsoleScreen(CLEAR_SCREEN_ALL);
            resetImg(img, imgColors, rowL, columnL);
            imgSurround(img, imgColors, rowL, columnL);
            printImg(img, imgColors, rowL, columnL);
            moveCursorTo(2, columnL + 6);
            printf("TOP  : %d", topScore);
            moveCursorTo(3, columnL + 6);
            printf("SCORE: %d", score);
            moveCursorTo(3, columnL / 3);
            printf("LINES: %d", totalClear);
            while(continueGame) {
                continueGame = slctTetro(rowL, columnL, &tetroColor, &tetroH, &tetroW, &startC, &skipR1, &skipC1, &skipR2, &skipC2);
                hideCursor();
                if(continueGame) {
                    continueGame = addTetro(img, imgColors, tetroColor, rowL, columnL, tetroH, tetroW, startC, skipR1, skipC1, skipR2, skipC2);
                    clearImgFromScreen(rowL, columnL);
                    printImg(img, imgColors, rowL, columnL);
                    if(!continueGame) {
                        // GAME OVER SCREEN

                        moveCursorTo(rowL + 5, 1);
                        setTextColor(CYAN);
                        printf("   _____          __  __ ______      ______      ________ _____          \n");
                        mySleep(50);
                        printf("  / ____|   /\\   |  \\/  |  ____|    / __ \\ \\    / /  ____|  __ \\    \n");
                        mySleep(50);
                        printf(" | |  __   /  \\  | \\  / | |__      | |  | \\ \\  / /| |__  | |__) |    \n");
                        mySleep(50);
                        printf(" | | |_ | / /\\ \\ | |\\/| |  __|     | |  | |\\ \\/ / |  __| |  _  /    \n");
                        mySleep(50);
                        printf(" | |__| |/ ____ \\| |  | | |____    | |__| | \\  /  | |____| | \\ \\     \n");
                        mySleep(50);
                        printf("  \\_____/_/    \\_\\_|  |_|______|    \\____/   \\/   |______|_|  \\_\\ \n");
                        mySleep(50);

                        setTextColor(YELLOW);
                        showCursor();
                        printf("\n               Enter any key to go to the main menu: ");
                        setTextColor(RESET_COLOR);
                        scanf(" %c", &scanfGarbage);
                    }

                    if(continueGame) {
                        botRow = tetroH;
                        while(obstacleControl(img, rowL, columnL, tetroW, tetroH, startC, botRow, skipR1, skipC1, skipR2, skipC2)) {
                            mySleep(DELAY);
                            tetroFall(img, imgColors, rowL, columnL, tetroH, tetroW, startC, botRow, skipR1, skipC1, skipR2, skipC2);
                            clearImgFromScreen(rowL, columnL);
                            printImg(img, imgColors, rowL, columnL);
                            mySleep(DELAY);
                            score += 2; // kaydigi satir basina 1 puan kazaniyor
                            botRow++;
                        }
                        clearCount = clearRow(img, imgColors, rowL, columnL, botRow, tetroH);
                        totalClear += clearCount;
                        moveCursorTo(3, columnL / 3);
                        printf("LINES: %d", totalClear);
                        switch(clearCount) {
                        case 1:
                            score += 80;
                            break;
                        case 2:
                            score += 200;
                            break;
                        case 3:
                            score += 600;
                            break;
                        case 4:
                            score += 2400;
                            break;
                        }
                        if(score > topScore) {
                            topScore = score;
                            moveCursorTo(2, columnL + 6);
                            printf("Top  : %d", topScore);
                            moveCursorTo(3, columnL + 6);
                            printf("Score: %d", score);
                        } else {
                            moveCursorTo(3, columnL + 6);
                            printf("Score: %d", score);
                        }
                    }
                }
                showCursor();
            }
            clearConsoleScreen(CLEAR_SCREEN_ALL);
        }
    }
    resetConsole();
    return 0;
}

void mySleep(int milliseconds) // programin girilen deger kadar milisaniye uyumasini saglayan fonksiyon
{
#ifdef _WIN32
    Sleep(milliseconds); // windows isletim sisteminde uyutma fonksiyonu
#endif
#ifndef _WIN32
    usleep(milliseconds * 1000); // UNIX isletim sistemlerinde uyutma fonksiyonu
#endif
}

void printImg(char img[M][M], int imgColors[M][M], int rowL, int columnL) // ekrana oyun ekranini basan fonksiyon
{
    int i, j;

    moveCursorTo(4, 1);
    for(i = 0; i < rowL; i++) {
        for(j = 0; j < columnL; j++) {
            if(img[i][j] != 0) {
                setTextColor(imgColors[i][j]); // dongunun bulundugu gozun rengi neyse renk ona ayarlaniyor
                printf("%c", img[i][j]);
                setTextColor(RESET_COLOR);
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }
}

void clearImgFromScreen(int rowL, int columnL)
{
    int i;
    for(i = 4; i < 4 + rowL; i++) {
        moveCursorTo(i, columnL + 1);
        clearConsoleLine(CLEAR_LINE_FROM_CURSOR_TO_BEGIN);
    }
}

void resetImg(char img[M][M], int imgColors[M][M], int rowL, int columnL) // oyun ekraninin tamamini sifirlayan fonksiyon
{
    int i, j;

    for(i = 0; i < rowL; i++) {
        for(j = 0; j < columnL; j++) {
            img[i][j] = 0;
            imgColors[i][j] = RESET_COLOR;
        }
    }
}

void imgSurround(char img[M][M], int imgColors[M][M], int rowL, int columnL) // oyun ekranina cerceve ekleyen fonksiyon
{
    int i;

    img[0][0] = '/';                  // sol ust kosedeki cizgi
    img[0][columnL - 1] = '\\';       // sol alt kosedeki cizgi
    img[rowL - 1][0] = '\\';          // sag ust kosedeki cizgi
    img[rowL - 1][columnL - 1] = '/'; // sag alt kosedeki cizgi
    for(i = 1; i < rowL - 1; i++) {
        img[i][0] = '|';           // ilk sutundaki dikey cizgiler
        img[i][columnL - 1] = '|'; // son sutundaki dikey cizgiler
    }
    for(i = 1; i < columnL - 1; i++) { // tavan ve taban
        if(i % 5 == 0) {               // 5'in katiysa belirtec olarak renklendir
            img[0][i] = '=';
            img[rowL - 1][i] = '=';

            imgColors[0][i] = RED;
            imgColors[rowL - 1][i] = RED;
        } else {
            img[0][i] = '=';        // tavandaki yatay cizgiler
            img[rowL - 1][i] = '='; // tabandaki yatay cizgiler
        }
    }
}

int addTetro(
    char img[M][M],
    int imgColors[M][M],
    int tetroColor,
    int rowL,
    int columnL,
    int tetroH,
    int tetroW,
    int startC,
    int skipR1,
    int skipC1,
    int skipR2,
    int skipC2)
{
    /* Kullanicidan aldigimiz genislik, yukseklik ve baslangic sutunu degerlerini
     * kullanarak dongulerin sinirlarini belirliyoruz. Tetrominolar dikdortgen degil de
     * girintili oldugu icin dongude tetrominoya ait olmayan i-j ikililerini atlamamiz gerekiyor.
     * Atlayacagimiz ikililer skipR1-skipC1 ve skipR2-skipC2 parametreleri olarak fonksiyona veriliyor.
     * Fonksiyon ayrica oyunun bitip bitmedigini de kontrol ediyor.
     * Yildiz koyulmasi gereken yer bossa yildiz koyulup sayac artiriliyor. 
     * Eger doluysa yildiz koyuyor fakat sayaci artirmiyor.
     * En son tetrominonun toplam blok sayisiyla sayaci karsilastiriyor.
     * Ikisi esitse fonksiyon 1 degerini degilse 0 degerini donduruyor.
     */
    int i, j, count = 0, numberOfBlocks = 0, continueGame;

    for(i = 1; i <= tetroH; i++) {
        for(j = startC; j < startC + tetroW; j++) {
            if((i != skipR1 || j != skipC1) && (i != skipR2 || j != skipC2)) {
                numberOfBlocks++;
                if(img[i][j] != '*') {
                    img[i][j] = '*';
                    imgColors[i][j] = tetroColor; // yildiz koyulan yerin rengini de ayarliyoruz
                    count++;
                } else { // doluysa sadece yerlestirme yapiliyor count artirilmiyor
                    img[i][j] = '*';
                    imgColors[i][j] = tetroColor;
                }
            }
        }
    }
    if(count == numberOfBlocks)
        continueGame = 1;
    else
        continueGame = 0;
    return continueGame;
}

int obstacleControl(
    char img[M][M],
    int rowL,
    int columnL,
    int tetroW,
    int tetroH,
    int startC,
    int botRow,
    int skipR1,
    int skipC1,
    int skipR2,
    int skipC2)
{
    /* Tetrominoya ait olmayan gozleri kontrol etmemek icin slctTetro fonksiyonunda belirlenen
     * skipR1, skipR2 degerlerini duzenleyip burada da kullanmamiz lazim.
     * Tetromino saga sola kaymayacagi icin skipC1 ve skipC2 degerleri ayni kalir.
     * skipR1, skipR2 degerlerini duzenleyebilmek icin kac satir asagi kaydigini bulmamiz lazim.
     * (botRow - tetroH) bize ilk yerlestigi konumdan kac satir asagi kaydigini verir.
     * Kontrol yaparken tetrominonun her sutunundaki en alt blogun alti bos mu diye kontrol edecegiz.
     * En alt bloklardan her alti bos olan icin count 1 artiriliyor.
     * En son count degeri ile tetrominonun genisligini karsilastiriyoruz.
     * Iki deger esitse fonksiyon 1 degeri donduruyor, degilse 0 donduruyor.
     */
    int count, i, freeToFall;

    skipR1 += (botRow - tetroH);
    skipR2 += (botRow - tetroH);
    count = 0;
    for(i = startC; i < startC + tetroW; i++) { // tetrominonun bulundugu sutunlari gezen dongu
        if(botRow != skipR1 || i != skipC1) {
            if(botRow != skipR2 || i != skipC2) {
                if(img[botRow + 1][i] == 0)
                    count++;
            } else {
                if((skipR2 - 1 == skipR1) && (skipC1 == skipC2)) {
                    if(img[skipR1][skipC1] == 0)
                        count++;
                } else {
                    if(img[skipR2][skipC2] == 0)
                        count++;
                }
            }
        } else {
            if((img[skipR1 - 1 == skipR2]) && (skipC1 == skipC2)) {
                if(img[skipR2][skipC2] == 0)
                    count++;
            } else {
                if(img[skipR1][skipC1] == 0)
                    count++;
            }
        }
    }
    if(count == tetroW)
        freeToFall = 1;
    else
        freeToFall = 0;
    return freeToFall;
}

void tetroFall(
    char img[M][M],
    int imgColors[M][M],
    int rowL,
    int columnL,
    int tetroH,
    int tetroW,
    int startC,
    int botRow,
    int skipR1,
    int skipC1,
    int skipR2,
    int skipC2)
{
    /* slctTetro fonksiyonunda belirledigimiz skipR1 ve skipR2 degerlerini tetrominonun bulundugu satira
     * gore yeniden duzenliyoruz. Kaydirma isleminde sadece tetrominoya ait olan gozleri
     * kaydiriyoruz. Her yildiz kaydirmadan sonra onceden bulundugu gozdeki yildizi siliyoruz.
     */
    int i, j;

    skipR1 += (botRow - tetroH);
    skipR2 += (botRow - tetroH);
    for(i = botRow; i > botRow - tetroH; i--) {
        for(j = startC; j < startC + tetroW; j++) {
            if((i != skipR1 || j != skipC1) && (i != skipR2 || j != skipC2)) {
                img[i + 1][j] = img[i][j]; // yildizi kaydiriyoruz
                img[i][j] = 0;             // eski yerindeki yildizi sifirliyoruz

                imgColors[i + 1][j] = imgColors[i][j]; // rengi kaydiriyoruz
                imgColors[i][j] = RESET_COLOR;         // eski yerindeki rengi sifirliyoruz
            }
        }
    }
}

int clearRow(char img[M][M], int imgColors[M][M], int rowL, int columnL, int botRow, int tetroH)
{
    /* Silinmesi gereken satirlara bakarken sadece tetrominonun en son yerlestigi
     * satirlara bakiyoruz cunku degisiklik sadece o satirlarda oldu. Dongu tetrominonun
     * yerlestigi en ust satirdan basliyor. Satirin tamami yildizdan olusuyorsa o satirin silinmesi gerek.
     * Silinecek satirlar birlesikse onlari tek seferde silip usttekileri topluca kaydirmak
     * gerekiyor. Bunu saglayabilmek icin silmeye baslayacagimiz satir startRow
     * degiskeninde, art arda satir sayisi seqRowCount degiskeninde saklaniyor. Eger tamami dolu satir
     * startRow'dan seqRowCount kadar buyukse onceki silinecek satirlarla bitisik demektir. Tamami dolu olmayan
     * bir satirla karsilastigimizda oncesinde ne kadar bitisik dolu satir varsa hepsi silinip ustlerindeki
     * satirlar silinen satir sayisi kadar asagi kaydiriliyor. Tetrominonun yerlestigi son satirin tamami doluysa
     * dongu bitecegi icin en son silinmesi gereken satirlar silinmeyecek. Dongu bittikten sonra seqRowCount degeri
     * sifirdan farkliysa silme ve kaydirma islemi gerceklestiriliyor.
     * Fonksiyon silinen toplam satir sayisini donduruyor.
     */
    int i, j, k, m, startRow = -1, seqRowCount = 0, fullRowCount = 0;

    for(i = botRow - tetroH + 1; i <= botRow; i++) {
        j = 1;
        while(img[i][j] == '*' && j < columnL - 1)
            j++;
        if(j == columnL - 1) {
            fullRowCount++;
            if(i == startRow + seqRowCount) {
                seqRowCount++;
            } else {
                startRow = i;
                seqRowCount = 1;
            }
        } else {
            if(seqRowCount != 0) {
                mySleep(CLEAR_DELAY);
                for(k = startRow; k < startRow + seqRowCount; k++) { // satirlari sil
                    for(m = 1; m < columnL - 1; m++) {
                        img[k][m] = 0;
                    }
                }
                clearImgFromScreen(rowL, columnL);
                printImg(img, imgColors, rowL, columnL);
                mySleep(CLEAR_DELAY);
                for(k = startRow - 1; k > 0; k--) { // kaydir
                    for(m = 1; m < columnL - 1; m++) {
                        img[k + seqRowCount][m] = img[k][m];
                        img[k][m] = 0;

                        imgColors[k + seqRowCount][m] = imgColors[k][m];
                        imgColors[k][m] = 0;
                    }
                }
                clearImgFromScreen(rowL, columnL);
                printImg(img, imgColors, rowL, columnL);
            }
            startRow = -1;
            seqRowCount = 0;
        }
    }
    if(seqRowCount != 0) {
        mySleep(CLEAR_DELAY);
        for(k = startRow; k < startRow + seqRowCount; k++) { // satirlari sil
            for(m = 1; m < columnL - 1; m++) {
                img[k][m] = 0;
            }
        }
        clearImgFromScreen(rowL, columnL);
        printImg(img, imgColors, rowL, columnL);
        mySleep(CLEAR_DELAY);
        for(k = startRow - 1; k > 0; k--) { // kaydir
            for(m = 1; m < columnL - 1; m++) {
                img[k + seqRowCount][m] = img[k][m];
                img[k][m] = 0;

                imgColors[k + seqRowCount][m] = imgColors[k][m];
                imgColors[k][m] = 0;
            }
        }
        clearImgFromScreen(rowL, columnL);
        printImg(img, imgColors, rowL, columnL);
    }
    return fullRowCount;
}

void rotateTetroRight(char rotation[R][R])
{
    /* Kare bir matriste dondurme islemi yapmak icin kare matrisi distan ice kare cemberlerden
     * olusuyor gibi dusunebiliriz. Ornegin 4x4 bir matris dista 4x4 luk icte 2x2 lik olmak uzere 2 cemberden
     * olusur.
     * 1  2  3  4    Dondurecegimiz matris olarak dusunursek, once en distaki cemberden baslariz.
     * 5  6  7  8    1 degeri tmp de saklanir, 13 1'in yerine gecer, 16 13'un yerine gecer, 4 16'nin yerine gecer
     * 9  10 11 12   en son olarak da tmp 4'un yerine gecer. Koseler tamamlandiktan sonra 2-12-14-5 arasinda kayma olur.
     * 13 14 15 16   Dis cember donduruldukten sonra ayni islemler icteki cembere uygulanir.
     */
    int i, j, tmp;
    for(i = 0; i < R / 2; i++) {
        for(j = i; j < R - 1 - i; j++) {
            tmp = rotation[i][j];
            rotation[i][j] = rotation[R - 1 - j][i];
            rotation[R - 1 - j][i] = rotation[R - 1 - i][R - 1 - j];
            rotation[R - 1 - i][R - 1 - j] = rotation[j][R - 1 - i];
            rotation[j][R - 1 - i] = tmp;
        }
    }
}

void rotateTetroLeft(char rotation[R][R])
{
    int i, j, tmp;
    for(i = 0; i < R / 2; i++) {
        for(j = i; j < R - 1 - i; j++) {
            tmp = rotation[i][j];
            rotation[i][j] = rotation[j][R - 1 - i];
            rotation[j][R - 1 - i] = rotation[R - 1 - i][R - 1 - j];
            rotation[R - 1 - i][R - 1 - j] = rotation[R - 1 - j][i];
            rotation[R - 1 - j][i] = tmp;
        }
    }
}

void printRotation(char rotation[R][R], int tetroColor, int rowL, int columnL) // ekrana dondurme ekranini basan fonksiyon
{
    int i, j, cursorRow = 5;

    hideCursor();
    setTextColor(tetroColor); // matrisi basacagimiz rengi ayarliyoruz
    moveCursorTo(cursorRow, columnL + 7);
    cursorRow++;
    for(i = 0; i < R; i++) {
        for(j = 0; j < R; j++) {
            if(rotation[i][j] != 0) {
                printf("%c", rotation[i][j]);
            } else {
                printf(" ");
            }
        }
        moveCursorTo(cursorRow, columnL + 7);
        cursorRow++;
    }
    setTextColor(RESET_COLOR);
    showCursor();
}

int slctTetro(
    int rowL,
    int columnL,
    int *ptr_tetroColor,
    int *ptr_tetroH,
    int *ptr_tetroW,
    int *ptr_startC,
    int *ptr_skipR1,
    int *ptr_skipC1,
    int *ptr_skipR2,
    int *ptr_skipC2)
{
    /* Siradaki tetromino rastgele belirlendikten sonra gelen parcayi rotation matrisine yerlestiriyoruz.
     * Kullanici oyunu bitirmek istiyorsa parca dondurme ekraninda 'e' tusuna basarak bitirebiliyor.
     * Kullanicinin istegine gore rotation matrisi uzerinde dondurme islemi yapiliyor.
     * Kullanici parcayi saga dondurdukce rotationNumber artarken sola dondurdukce azaliyor.
     * rotationNumber'a negatif ve pozitif sayilarda mod almaya yarayan   A-> parcanin rotasyon sayisi
     * [(rotationNumber % A + A) % A] islemini uyguladigimizda kullanicinin en son durdugu rotasyonun
     * kac numarali rotasyon oldugunu bulabiliyoruz. Kullanici rotasyonu sectikten sonra startC degerini giriyor.
     * Eger girdigi startC degeri ekranin sagindan tasacak bir degerse tetrominoyu ekranin sagina dayamak istemistir
     * diye dusunup startC'yi bu sekilde duzenliyoruz. Daha sonra secilen rotasyona gore tetrominonun
     * genislik, yukseklik ve tetrominoya ait olmayan bloklari belirleyen skipR1,skipC1,skipR2,skipC2
     * degerlerini main fonksiyonuna pointer yardimi ile iletiyoruz. Kullanici parca secimini gerceklestirdiyse
     * yani oyunu bitirmek istemiyorsa 1 degeri, aksi halde 0 degeri donduruluyor.
     */

    int slct;                        // hangi tetrominonun gelecegi
    int rotationNumber = 0;          // kullanicinin hangi rotasyonda durdugunu anlamamiza yarayan degisken
    int slctRot;                     // tetrominonun hangi rotasyonda oldugu
    int tetroColor;                  // tetromino rengi
    int continueGame = 1;            // oyunun devam edip etmeyecegini belirleyen degisken
    char rotation[R][R] = { { 0 } }; // dondurme yapilan matris
    char pickRotateOrExit;

    slct = rand() % 7;
    switch(slct) {
    case 0:
        rotation[1][0] = rotation[1][1] = rotation[1][2] = rotation[1][3] = '*'; // I
        *ptr_tetroColor = CYAN;                                                  // rengi ayarliyoruz
        break;
    case 1:
        rotation[1][1] = rotation[2][1] = rotation[2][2] = rotation[2][3] = '*'; // J
        *ptr_tetroColor = BLUE;
        break;
    case 2:
        rotation[2][1] = rotation[2][2] = rotation[2][3] = rotation[1][3] = '*'; // L
        *ptr_tetroColor = YELLOW;
        break;
    case 3:
        rotation[1][1] = rotation[1][2] = rotation[2][1] = rotation[2][2] = '*'; // O
        *ptr_tetroColor = WHITE;
        break;
    case 4:
        rotation[2][1] = rotation[2][2] = rotation[1][2] = rotation[1][3] = '*'; // S
        *ptr_tetroColor = GREEN;
        break;
    case 5:
        rotation[1][2] = rotation[2][1] = rotation[2][2] = rotation[2][3] = '*'; // T
        *ptr_tetroColor = MAGENTA;
        break;
    case 6:
        rotation[1][1] = rotation[1][2] = rotation[2][2] = rotation[2][3] = '*'; // Z
        *ptr_tetroColor = RED;
        break;
    }
    tetroColor = *ptr_tetroColor;
    do {
        moveCursorTo(14, columnL + 6);
        clearConsoleLine(CLEAR_LINE_FROM_CURSOR_TO_END); // onceki startC secme yazisini siliyoruz
        printRotation(rotation, tetroColor, rowL, columnL);
        moveCursorTo(10, columnL + 5);
        setTextColor(CYAN);
        printf(" [Z] [X]  ");
        setTextColor(RESET_COLOR);
        printf(":Rotate");
        moveCursorTo(11, columnL + 5);
        setTextColor(YELLOW);
        printf("   [C]    ");
        setTextColor(RESET_COLOR);
        printf(":Pick              ( )");
        moveCursorTo(12, columnL + 5);
        setTextColor(RED);
        printf("   [E]    ");
        setTextColor(RESET_COLOR);
        printf(":Back to the main menu");
        moveCursorTo(11, columnL + 35);
        scanf(" %c", &pickRotateOrExit);

        if(pickRotateOrExit != 'c' || pickRotateOrExit != 'C') {
            if(pickRotateOrExit == 'x' || pickRotateOrExit == 'X') {
                rotateTetroRight(rotation);
                rotationNumber++;
            } else if(pickRotateOrExit == 'z' || pickRotateOrExit == 'Z') {
                rotateTetroLeft(rotation);
                rotationNumber--;
            }
        }
    } while(pickRotateOrExit != 'c' && pickRotateOrExit != 'e' && pickRotateOrExit != 'C' && pickRotateOrExit != 'E');

    if(pickRotateOrExit != 'e' && pickRotateOrExit != 'E') { // kullanici oyunu bitirmek istemiyorsa parcayi yerlestirmeye gec
        moveCursorTo(14, columnL + 6);
        printf("Column to place tetromino: ");
        scanf("%d", &*ptr_startC);

        switch(slct) {
        case 0:
            slctRot = (rotationNumber % 2 + 2) % 2;
            switch(slctRot) {
            case 0:
                *ptr_tetroW = 4;
                *ptr_tetroH = 1;
                if(*ptr_startC + *ptr_tetroW - 1 >= (columnL - 1))          // --> Kullanici ekranin sagindan tasacak
                    *ptr_startC = (columnL - 1) - *ptr_tetroW;              // bir startC girdiyse onu saga hizaliyoruz.
                *ptr_skipR1 = *ptr_skipR2 = *ptr_skipC1 = *ptr_skipC2 = -1; // --> Parca girintili degilse -1 atiyoruz
                break;
            case 1:
                *ptr_tetroW = 1;
                *ptr_tetroH = 4;
                if(*ptr_startC + *ptr_tetroW - 1 >= (columnL - 1))
                    *ptr_startC = (columnL - 1) - *ptr_tetroW;
                *ptr_skipR1 = *ptr_skipR2 = *ptr_skipC1 = *ptr_skipC2 = -1;
                break;
            }
            break;
        case 1:
            slctRot = (rotationNumber % 4 + 4) % 4;
            switch(slctRot) {
            case 0:
                *ptr_tetroW = 3;
                *ptr_tetroH = 2;
                if(*ptr_startC + *ptr_tetroW - 1 >= (columnL - 1))
                    *ptr_startC = (columnL - 1) - *ptr_tetroW;
                *ptr_skipR1 = 1, *ptr_skipC1 = *ptr_startC + 1;
                *ptr_skipR2 = 1, *ptr_skipC2 = *ptr_startC + 2;
                break;
            case 1:
                *ptr_tetroW = 2;
                *ptr_tetroH = 3;
                if(*ptr_startC + *ptr_tetroW - 1 >= (columnL - 1))
                    *ptr_startC = (columnL - 1) - *ptr_tetroW;
                *ptr_skipR1 = 2, *ptr_skipC1 = *ptr_startC + 1;
                *ptr_skipR2 = 3, *ptr_skipC2 = *ptr_startC + 1;
                break;
            case 2:
                *ptr_tetroW = 3;
                *ptr_tetroH = 2;
                if(*ptr_startC + *ptr_tetroW - 1 >= (columnL - 1))
                    *ptr_startC = (columnL - 1) - *ptr_tetroW;
                *ptr_skipR1 = 2, *ptr_skipC1 = *ptr_startC;
                *ptr_skipR2 = 2, *ptr_skipC2 = *ptr_startC + 1;
                break;
            case 3:
                *ptr_tetroW = 2;
                *ptr_tetroH = 3;
                if(*ptr_startC + *ptr_tetroW - 1 >= (columnL - 1))
                    *ptr_startC = (columnL - 1) - *ptr_tetroW;
                *ptr_skipR1 = 1, *ptr_skipC1 = *ptr_startC;
                *ptr_skipR2 = 2, *ptr_skipC2 = *ptr_startC;
                break;
            }
            break;
        case 2:

            slctRot = (rotationNumber % 4 + 4) % 4;
            switch(slctRot) {
            case 0:
                *ptr_tetroW = 3;
                *ptr_tetroH = 2;
                if(*ptr_startC + *ptr_tetroW - 1 >= (columnL - 1))
                    *ptr_startC = (columnL - 1) - *ptr_tetroW;
                *ptr_skipR1 = 1, *ptr_skipC1 = *ptr_startC;
                *ptr_skipR2 = 1, *ptr_skipC2 = *ptr_startC + 1;
                break;
            case 1:
                *ptr_tetroW = 2;
                *ptr_tetroH = 3;
                if(*ptr_startC + *ptr_tetroW - 1 >= (columnL - 1))
                    *ptr_startC = (columnL - 1) - *ptr_tetroW;
                *ptr_skipR1 = 1, *ptr_skipC1 = *ptr_startC + 1;
                *ptr_skipR2 = 2, *ptr_skipC2 = *ptr_startC + 1;
                break;
            case 2:
                *ptr_tetroW = 3;
                *ptr_tetroH = 2;
                if(*ptr_startC + *ptr_tetroW - 1 >= (columnL - 1))
                    *ptr_startC = (columnL - 1) - *ptr_tetroW;
                *ptr_skipR1 = 2, *ptr_skipC1 = *ptr_startC + 1;
                *ptr_skipR2 = 2, *ptr_skipC2 = *ptr_startC + 2;
                break;
            case 3:
                *ptr_tetroW = 2;
                *ptr_tetroH = 3;
                if(*ptr_startC + *ptr_tetroW - 1 >= (columnL - 1))
                    *ptr_startC = (columnL - 1) - *ptr_tetroW;
                *ptr_skipR1 = 2, *ptr_skipC1 = *ptr_startC;
                *ptr_skipR2 = 3, *ptr_skipC2 = *ptr_startC;
                break;
            }
            break;
        case 3:

            *ptr_tetroW = 2;
            *ptr_tetroH = 2;
            if(*ptr_startC + *ptr_tetroW - 1 >= (columnL - 1))
                *ptr_startC = (columnL - 1) - *ptr_tetroW;
            *ptr_skipR1 = *ptr_skipR2 = *ptr_skipC1 = *ptr_skipC2 = -1;
            break;
        case 4:

            slctRot = (rotationNumber % 2 + 2) % 2;
            switch(slctRot) {
            case 0:
                *ptr_tetroW = 3;
                *ptr_tetroH = 2;
                if(*ptr_startC + *ptr_tetroW - 1 >= (columnL - 1))
                    *ptr_startC = (columnL - 1) - *ptr_tetroW;
                *ptr_skipR1 = 1, *ptr_skipC1 = *ptr_startC;
                *ptr_skipR2 = 2, *ptr_skipC2 = *ptr_startC + 2;
                break;
            case 1:
                *ptr_tetroW = 2;
                *ptr_tetroH = 3;
                if(*ptr_startC + *ptr_tetroW - 1 >= (columnL - 1))
                    *ptr_startC = (columnL - 1) - *ptr_tetroW;
                *ptr_skipR1 = 1, *ptr_skipC1 = *ptr_startC + 1;
                *ptr_skipR2 = 3, *ptr_skipC2 = *ptr_startC;
                break;
            }
            break;
        case 5:

            slctRot = (rotationNumber % 4 + 4) % 4;
            switch(slctRot) {
            case 0:
                *ptr_tetroW = 3;
                *ptr_tetroH = 2;
                if(*ptr_startC + *ptr_tetroW - 1 >= (columnL - 1))
                    *ptr_startC = (columnL - 1) - *ptr_tetroW;
                *ptr_skipR1 = 1, *ptr_skipC1 = *ptr_startC;
                *ptr_skipR2 = 1, *ptr_skipC2 = *ptr_startC + 2;
                break;
            case 1:
                *ptr_tetroW = 2;
                *ptr_tetroH = 3;
                if(*ptr_startC + *ptr_tetroW - 1 >= (columnL - 1))
                    *ptr_startC = (columnL - 1) - *ptr_tetroW;
                *ptr_skipR1 = 1, *ptr_skipC1 = *ptr_startC + 1;
                *ptr_skipR2 = 3, *ptr_skipC2 = *ptr_startC + 1;
                break;
            case 2:
                *ptr_tetroW = 3;
                *ptr_tetroH = 2;
                if(*ptr_startC + *ptr_tetroW - 1 >= (columnL - 1))
                    *ptr_startC = (columnL - 1) - *ptr_tetroW;
                *ptr_skipR1 = 2, *ptr_skipC1 = *ptr_startC;
                *ptr_skipR2 = 2, *ptr_skipC2 = *ptr_startC + 2;
                break;
            case 3:
                *ptr_tetroW = 2;
                *ptr_tetroH = 3;
                if(*ptr_startC + *ptr_tetroW - 1 >= (columnL - 1))
                    *ptr_startC = (columnL - 1) - *ptr_tetroW;
                *ptr_skipR1 = 1, *ptr_skipC1 = *ptr_startC;
                *ptr_skipR2 = 3, *ptr_skipC2 = *ptr_startC;
                break;
            }
            break;
        case 6:

            slctRot = (rotationNumber % 2 + 2) % 2;
            switch(slctRot) {
            case 0:
                *ptr_tetroW = 3;
                *ptr_tetroH = 2;
                if(*ptr_startC + *ptr_tetroW - 1 >= (columnL - 1))
                    *ptr_startC = (columnL - 1) - *ptr_tetroW;
                *ptr_skipR1 = 1, *ptr_skipC1 = *ptr_startC + 2;
                *ptr_skipR2 = 2, *ptr_skipC2 = *ptr_startC;
                break;
            case 1:
                *ptr_tetroW = 2;
                *ptr_tetroH = 3;
                if(*ptr_startC + *ptr_tetroW - 1 >= (columnL - 1))
                    *ptr_startC = (columnL - 1) - *ptr_tetroW;
                *ptr_skipR1 = 1, *ptr_skipC1 = *ptr_startC;
                *ptr_skipR2 = 3, *ptr_skipC2 = *ptr_startC + 1;
                break;
            }
            break;
        }
    } else if(pickRotateOrExit == 'e' || pickRotateOrExit == 'E') {
        continueGame = 0;
    }
    return continueGame;
}

// Konsolla etkilesim fonksiyonlari

void setTextColor(int code)
{
    printf("\x1b[%dm", code);
}

void clearConsoleLine(int code)
{
    printf("\x1b[%dK", code);
}

void clearConsoleScreen(int code)
{
    printf("\x1b[%dJ", code);
}

void moveCursorTo(int row, int column)
{
    printf("\x1b[%d;%dH", row, column);
}

void moveCursorToBeginning()
{
    printf("\x1b[H");
}

void saveCursorPosition()
{
    printf("\x1b%d", 7);
}

void restoreCursorPosition()
{
    printf("\x1b%d", 8);
}
void hideCursor()
{
    printf("\x1b[?%dl", 25);
}

void showCursor()
{
    printf("\x1b[?%dh", 25);
}

#if defined(_WIN32) && !defined(ENABLE_VIRTUAL_TERMINAL_PROCESSING)
// windows sistemdeysek ve ENABLE_VIRTUAL_TERMINAL_PROCESSING compiler tarafindan tanimlanmadiysa
// biz tanimliyoruz
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

#ifdef _WIN32 // windows icin gerceklestirilecek islemler
static HANDLE stdoutHandle;
static DWORD outModeInit;

void setupConsole()
{
    /* ANSI Escape kodlari Linux ve macOS'te varsayilan olarak calisirken
     * w10 konsolunun ANSI Escape kodlarini kabul edecek sekilde ayarlanmasi gerekiyor.
     * Bu fonksiyon W10 konsolunu ayarliyor.
     */
    DWORD outMode = 0;
    stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if(stdoutHandle == INVALID_HANDLE_VALUE) {
        exit(GetLastError());
    }
    if(!GetConsoleMode(stdoutHandle, &outMode)) {
        exit(GetLastError());
    }
    outModeInit = outMode;
    // ANSI Escape kodlarini aktiflestirme
    outMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if(!SetConsoleMode(stdoutHandle, outMode)) {
        exit(GetLastError());
    }
}

void resetConsole() // konsolu varsayilan hale donduren fonksiyon
{
    // Konsolun rengini degistirip yazimizi yazdiktan sonra rengi resetliyoruz
    printf("\x1b[0m");
    // Konsol modunu resetliyoruz
    if(!SetConsoleMode(stdoutHandle, outModeInit)) {
        exit(GetLastError());
    }
}

#else // windows disindaki isletim sistemleri icin gerceklestirilecek islemler
void setupConsole()
{
}

void resetConsole()
{
    // Konsolun rengini degistirip yazimizi yazdiktan sonra rengi resetliyoruz
    printf("\x1b[0m");
}
#endif
