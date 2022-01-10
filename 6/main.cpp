#include <iostream>
#include <fstream>
#include <algorithm> //used for min() and max()
using namespace std;

class ImageCompression{
    public:
        int numRows, 
            numCols, 
            minVal, 
            maxVal, 
            newMinVal,
            newMaxVal,
            rowSize,
            colSize;

        int **skeletonArr, **zfArr;
        
    
    public:
        ImageCompression( ifstream &imgFile ){
            read_header(imgFile);
            this -> newMaxVal = 0;
            this -> newMinVal = 99999;
            this -> rowSize = this -> numRows + 2;
            this -> colSize = this -> numCols + 2;
            this -> zfArr = new int* [this -> rowSize];
            this -> skeletonArr = new int* [this -> rowSize];

            // create 2D array
            for(int i = 0; i < this -> rowSize; i++){
                this -> zfArr[i] = new int [this -> colSize];
                this -> skeletonArr[i] = new int [this -> colSize];
            }
        } 
        
        void read_header( ifstream &inFile ){
            inFile >> this -> numRows >> this -> numCols >> this -> minVal >> this -> maxVal ;
        }

        void setZero(int** arr){
            for(int i = 0; i < this -> rowSize; i++){
                for(int j = 0; j < this -> colSize; j++){
                    arr[i][j] = 0;
                }
            }
        }

        void loadImage(ifstream &inFile, int** arr){
            for(int i = 1; i < this -> numRows + 1; i++){
                for(int j = 1; j < this -> numCols + 1; j++){{
                    inFile >> arr[i][j];
                }}
            }
        }

        void compute8Distance(int** arr, ofstream &output){
            firstPass8Distance(arr);
            output << "Distance Transform: 1st Pass \n";
            reformatPrettyPrint(arr, output);
            secondPass8Distance(arr);
            output << "Distance Transform: 2nd Pass \n";
            reformatPrettyPrint(arr, output);
        }

        void firstPass8Distance(int** arr){
            // step 1 & 3:
            for(int i = 1; i < this -> numRows + 1; i++){
                for(int j = 1; j < this -> numCols + 1; j++){
                    int p = arr[i][j];

                    // step 2:
                    if(p > 0){
                        int a = arr[i-1][j-1];
                        int b = arr[i-1][j];
                        int c = arr[i-1][j+1];
                        int d = arr[i][j-1];

                        arr[i][j] = min( min(a,b), min(c,d) ) + 1;
                    }
                }
            }
        }

        void secondPass8Distance(int** arr){
            // Step 1 & 3:
            for(int i = this -> numRows; i > 0; i--){
                for(int j = this -> numCols; j > 0; j--){
                    int p = arr[i][j];
                    // step 2:
                    if(p > 0){
                        int e = arr[i][j+1];
                        int f = arr[i+1][j-1];
                        int g = arr[i+1][j];
                        int h = arr[i+1][j+1];

                        arr[i][j] = min ( min( min(e + 1, f + 1), min(g + 1,h + 1) ), p );
                    }

                }
            }
        }

        void imageCompression(int** zfArr, int** skeletonArr, ofstream &skeletonOutput, ofstream &output){
            computeLocalMaxima(zfArr, skeletonArr);
            output << "After computing local maxima, skeleton array \n";
            reformatPrettyPrint(skeletonArr, output);
            extractSkeleton(skeletonArr, skeletonOutput);
        }

        void computeLocalMaxima(int** zfArr, int** skeletonArr){
            // Step 1 & 3:
            for(int i = 1; i < this -> numRows + 1; i++){
                for(int j = 1; j < this -> numCols + 1; j++){

                    // Step 2:
                    int p = zfArr[i][j];
                    int a = zfArr[i-1][j-1];
                    int b = zfArr[i-1][j];
                    int c = zfArr[i-1][j+1];
                    int d = zfArr[i][j-1]; 
                    int e = zfArr[i][j+1];
                    int f = zfArr[i+1][j-1];
                    int g = zfArr[i+1][j];
                    int h = zfArr[i+1][j+1];
                    int neighbors[8] = {a,b,c,d,e,f,g,h};

                    // isLocalMaxima
                    bool flag = true;
                    for(int k = 0; k < 9; k++){
                        if(p >= neighbors[k]) continue;
                        else flag = false;
                    }
                            
                    if( flag ){
                        skeletonArr[i][j] = p;
                        if(p > this -> newMaxVal) this -> newMaxVal = p;
                        else if( p < this -> newMinVal ) this -> newMinVal = p;
                    } 
                    else {
                        skeletonArr[i][j] = 0;
                        this -> newMinVal = 0;
                    }
                    
                }
            }
        }

        void loadSkeleton(ifstream &inFile, int** arr){
            read_header(inFile);
            int r, c, val;
            inFile >> r >> c >> val;

            for(int i = 0; i < this -> rowSize; i++){
                for(int j = 0; j < this -> colSize; j++){
                    if(i == r && j == c){
                        arr[i][j] = val;
                        inFile >> r >> c >> val;
                    }
                    else arr[i][j] = 0;
                }
            }
        }

        void extractSkeleton(int** arr, ofstream &output){
            output << this -> numRows << " " << this -> numCols << " " << this -> newMinVal << " " << this -> newMaxVal << endl;  
            for(int i = 1; i < this -> numRows + 1; i++){
                for(int j = 1; j < this -> numCols + 1; j++){
                    if( arr[i][j] > 0 ) output << i << " " << j << " " << arr[i][j] << endl;
                }
            }
        }

        void reformatPrettyPrint(int** arr, ofstream &output){
            for(int i = 0; i < this -> rowSize; i++){
                for(int j = 0; j < this -> colSize; j++){
                    output << arr[i][j] << " ";
                }
                output << endl;
            }
            output << endl;
        }

        void imageDecompression(int** arr, ofstream &output){
            firstPassExpansion(arr);
            output << "Image Decompression: Expansion Pass 1 \n";
            reformatPrettyPrint(arr, output);
            secondPassExpansion(arr);
            output << "Image Decompression: Expansion Pass 2 \n";
            reformatPrettyPrint(zfArr, output);
        }

        void firstPassExpansion(int** arr){
            for(int i = 1; i < this -> numRows; i++){
                for(int j = 1; j < this -> numCols; j++){
                    int p = arr[i][j];
                    if(p == 0){
                        int a = zfArr[i-1][j-1];
                        int b = zfArr[i-1][j];
                        int c = zfArr[i-1][j+1];
                        int d = zfArr[i][j-1]; 
                        int e = zfArr[i][j+1];
                        int f = zfArr[i+1][j-1];
                        int g = zfArr[i+1][j];
                        int h = zfArr[i+1][j+1];
                        int maximum;
                        maximum = max( 
                                        max( max(a,b), max(c,d) ),
                                        max( max(e,f), max(g,h) )
                                    ) - 1;
                        if(maximum < 0) arr[i][j] = 0;
                        else arr[i][j] = maximum;
                    }
                }
            }
        }

        void secondPassExpansion(int** arr){
            this -> newMaxVal = 0;
            this -> newMinVal = 999;
            for(int i = this -> numRows; i > 0; i--){
                for(int j = this -> numCols; j > 0; j--){
                    int p = arr[i][j];
                    int a = arr[i-1][j-1];
                    int b = arr[i-1][j];
                    int c = arr[i-1][j+1];
                    int d = arr[i][j-1]; 
                    int e = arr[i][j+1];
                    int f = arr[i+1][j-1];
                    int g = arr[i+1][j];
                    int h = arr[i+1][j+1];
                    
                    int maximum;
                    maximum = max(
                                max( 
                                    max( max(a,b), max(c,d) ),
                                    max( max(e,f), max(g,h) )
                                ) - 1,
                                p
                            );
                    
                    if (p > this -> newMaxVal) this -> newMaxVal = p;
                    if (p < this -> newMinVal) this -> newMinVal = p;
                    if(p < maximum) arr[i][j] = maximum;
                }
            }
        }

        void threshold(int** arr, ofstream &output){
            for(int i = 1; i < this -> numRows + 1; i++){
                for(int j = 1; j < this -> numCols + 1; j++){
                    int p = arr[i][j];
                    if(p > 0) output << 1 << " ";
                    else output << 0 << " ";
                }
                output << endl;
            }
        }
};

int main( int argc, const char * argv[] ) {

    if(argc != 3){
        cout << "Invalid amount of arguments";
        exit(1);
    }

    string imgFile = argv[1] ; 
    string outFile = argv[ 2 ] ; 
    ifstream  imgStream, inSkeletonStream; 
    ofstream outStream, outSkeletonStream, decompressedStream; 
    imgStream.open( imgFile ) ;
    outStream.open( outFile ) ;
  
    if( imgStream.is_open() ){
            // step 0:
            ImageCompression* imageObj = new ImageCompression( imgStream );
            // step 1 & 2:
            string skeletonFile = imgFile + "_skeleton.txt";
            outSkeletonStream.open( skeletonFile );
            // step 3 & 4:
            string decompressedFile = imgFile + "_decompressed.txt"; 
            decompressedStream.open( decompressedFile );
            // step 5:
            imageObj -> setZero(imageObj -> zfArr);
            imageObj -> setZero(imageObj -> skeletonArr);            
            // step 6:
            imageObj -> loadImage(imgStream, imageObj -> zfArr);
            outStream << "Original Image \n"; 
            imageObj -> reformatPrettyPrint(imageObj -> zfArr, outStream); 
            // step 7:
            imageObj -> compute8Distance(imageObj -> zfArr, outStream);
            // step 8:
            imageObj -> imageCompression(imageObj -> zfArr, imageObj -> skeletonArr, outSkeletonStream, outStream);
            // step 9, 10, 11:
            outSkeletonStream.close();
            inSkeletonStream.open(skeletonFile);
            imageObj -> setZero(imageObj -> zfArr);
            // step 12:
            imageObj -> loadSkeleton(inSkeletonStream, imageObj -> zfArr);
            outStream << "loaded from skeleton \n";
            imageObj -> reformatPrettyPrint(imageObj -> zfArr, outStream); 
            // step 13:
            imageObj -> imageDecompression(imageObj -> zfArr, outStream);
            // step 14:
            decompressedStream << imageObj -> numRows << " " << imageObj -> numCols << " " << 0  << " " << 1 << " " << endl; //binary image so, i will put 0 and 1
            // step 15:
            imageObj -> threshold(imageObj -> zfArr, decompressedStream);
            // step 16:
            imgStream.close() ;
            outStream.close();
            inSkeletonStream.close();
            decompressedStream.close();
            delete imageObj ;
    }
    else{ 
        cout << "Error: Input" << endl ; 
    }

    return 0;
}