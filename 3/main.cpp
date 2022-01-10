#include <iostream>
#include <fstream>
using namespace std;

class Morphology{
    public:
        int numImgRows, 
            numImgCols, 
            imgMin, 
            imgMax, 
            numStructRows, 
            numStructCols, 
            structMin, 
            structMax, 
            rowOrigin, 
            colOrigin,
            rowFrameSize,
            colFrameSize,
            extraRows,
            extraCols,
            rowSize,
            colSize,
            msgCounter;

        int **zeroFramedAry, **morphAry, **tempAry, **structAry;
        
    
    public:
        Morphology( ifstream &imgFile, ifstream &structFile ){
            read_img_header( imgFile );
            read_struct_header( structFile );
            read_origin( structFile );

            this -> rowFrameSize = this -> numStructRows / 2;
            this -> colFrameSize  = this -> numStructCols / 2;

            this -> extraRows = this -> rowFrameSize * 2;
            this -> extraCols = this -> colFrameSize * 2;
            this -> rowSize = this -> numImgRows + this -> extraRows;
            this -> colSize = this -> numImgCols + this -> extraCols;

            this -> zeroFramedAry = new int*[ rowSize ];
            this -> morphAry = new int*[ rowSize ];
            this -> tempAry = new int*[ rowSize ];

            for(int i = 0; i < this -> rowSize; i++){
                zeroFramedAry[i] = new int[ this -> colSize ];
                morphAry[i] = new int[ this -> colSize ];
                tempAry[i] = new int[ this -> colSize ];
            }

            this -> structAry = new int*[ this -> numStructRows ];

            for(int i = 0; i < this -> numStructRows; i++){
                structAry[i] = new int[ this -> numStructCols ]; 
            }

            this -> msgCounter = 0;
        } 
        
        void read_img_header( ifstream &inFile ){
            inFile >> this -> numImgRows >> this -> numImgCols >> this -> imgMin >> this -> imgMax ;
        }

        void read_struct_header( ifstream &inFile ){
            inFile >> this -> numStructRows >> this -> numStructCols >> this -> structMin >> this -> structMax ;
        }

        void read_origin( ifstream &inFile ){
            inFile >> this -> rowOrigin >> this -> colOrigin;
        }

        void zero2DAry(int **ary, int nRows, int nCols){
            for(int i = 0; i < nRows; i++){
                for(int j = 0; j < nCols; j++){
                    ary[i][j] = 0;
                }
            }
        }

        void loadImg( ifstream &inFile, int **ary){
            for( int i = this -> rowOrigin; i < this -> numImgRows; i++){
                for( int j = this -> colOrigin; j < this -> numImgCols; j++){
                    inFile >> ary[i][j];  
                }
            }
        }

        void loadStruct( ifstream &inFile, int **ary){
            for( int i = 0; i < this -> numStructRows; i++ ){
                for( int j = 0; j < this -> numStructCols; j++){
                    inFile >> ary[i][j];
                }
            }
        }

        void computeDilation( int **inAry, int **outAry){
            for( int i = this -> rowFrameSize; i < this -> rowSize; i++ ){
                for(int j = this -> colFrameSize; j < this -> colSize; j++ ){
                    if( inAry[i][j] > 0 ) this -> onePixelDilation(i,j, inAry, outAry);
                }
            }
        }

        void computeErosion( int **inAry, int **outAry ){
            for( int i = this -> rowFrameSize; i < this -> rowSize; i++ ){
                for(int j = this -> colFrameSize; j < this -> colSize; j++ ){
                    if( inAry[i][j] > 0 ) this -> onePixelErosion(i,j, inAry, outAry);
                }
            }
        }

        void computeOpening( int **inAry, int **outAry, int **tmp){
            this -> computeErosion(inAry, tmp);
            this -> computeDilation(tmp, outAry);
        }

        void computeClosing( int **inAry, int **outAry, int **tmp){
            this -> computeDilation(inAry, tmp);
            this -> computeErosion(tmp, outAry);
        }

        void onePixelDilation(int i, int j, int **inAry, int **outAry){
            int structRow = 0;
            int structCol = 0;

            for( int r = i - 1; r <= i + 1; r++ ){
                for( int c = j - 1; c <= j + 1; c++ ){
                    if(this -> structAry[ structRow ][ structCol ] == 1){
                        outAry[r][c] = this -> structAry[ structRow ][ structCol ];
                    }
                    structCol++;
                }
                structCol = 0;
                structRow++;
            }

        }

        void onePixelErosion(int i, int j, int **inAry, int **outAry){
            int iOffset = i - this -> rowOrigin;
            int jOffset = j - this -> colOrigin;
            bool matchFlag = true;
            int rIndex = 0;
            int cIndex = 0;

            while( matchFlag == true && rIndex < this -> numStructRows){
                cIndex = 0;
                while( matchFlag == true && cIndex < this -> numStructCols){
                    if( this -> structAry[rIndex][cIndex] > 0 && inAry[ iOffset + rIndex ][ jOffset + cIndex ] <= 0 ) matchFlag = false;
                    cIndex++;
                }
                rIndex++;
            }

            if( matchFlag == true ) outAry[i][j] = 1;
            else outAry[i][j] = 0;

        }

        void aryToFile(int **ary, ofstream &outFile ){
            outFile << this -> numImgRows << " " << this -> numImgCols << " " << this -> imgMin << " " << this -> imgMax << endl;
            for( int i = this -> rowFrameSize; i < this -> rowSize; i++ ){
                for( int j = this -> colFrameSize; j < this -> colSize; j++ ){
                    outFile << ary[i][j] << " ";
                }
                outFile << endl;
            }
        }

        void prettyPrint( int **ary,  ofstream &outFile){
            if(ary == this -> structAry){
                outFile << "Structure Element" << endl;
                for( int i = 0; i < this -> numStructRows; i++ ){
                    for( int j = 0; j < this -> numStructCols; j++ ){
                        if(ary[i][j] == 0) outFile << ". ";
                        else outFile << ary[i][j] << " ";
                    }
                    outFile << endl;
                }
            }else{
                string msg[5] = {"Zero Framed", "Dilation", "Erosion", "Opening", "Closing"};
                outFile << msg[this -> msgCounter] << endl;
                this -> msgCounter++;

                for( int i = 0; i < this -> rowSize; i++ ){
                    for( int j = 0; j < this -> colSize; j++ ){
                        if(ary[i][j] == 0) outFile << ". ";
                        else outFile << ary[i][j] << " ";
                    }
                    outFile << endl;
                }
            }
        }
};

int main( int argc, const char * argv[] ) {

    if(argc != 8){
        cout << "Invalid amount of arguments";
        exit(1);
    }

    string imgFile = argv[ 1 ] ; 
    string structFile = argv[ 2 ]; 
    ifstream  imgStream, structStream ; 

    imgStream.open( imgFile ) ;
    structStream.open( structFile );

    string dilateOutFile = argv[ 3 ] ; 
    string erodeOutFile = argv[ 4 ];
    string closingOutFile = argv[ 5 ];
    string openingOutFile = argv[ 6 ];
    string prettyPrintFile = argv[ 7 ];

    ofstream dilateStream, erodeStream, closingStream, openingStream, prettyPrintStream ; 
    dilateStream.open( dilateOutFile ) ;
    erodeStream.open( erodeOutFile );
    closingStream.open( closingOutFile );
    openingStream.open( openingOutFile );
    prettyPrintStream.open( prettyPrintFile );

    if( imgStream.is_open() && structStream.is_open() ){
            // step 1 & 2
            Morphology* morphObj = new Morphology( imgStream, structStream );

            // step 3
            morphObj -> zero2DAry(morphObj -> zeroFramedAry, morphObj -> rowSize, morphObj -> colSize );

            // step 4
            morphObj -> loadImg( imgStream, morphObj -> zeroFramedAry );
            morphObj -> prettyPrint( morphObj -> zeroFramedAry, prettyPrintStream );

            // step 5
            morphObj -> zero2DAry( morphObj -> structAry, morphObj -> numStructRows, morphObj -> numStructCols );
            morphObj -> loadStruct( structStream, morphObj -> structAry );
            morphObj -> prettyPrint( morphObj -> structAry, prettyPrintStream);

            // step 6
            morphObj -> zero2DAry( morphObj -> morphAry, morphObj -> rowSize, morphObj -> colSize );
            morphObj -> computeDilation( morphObj -> zeroFramedAry, morphObj -> morphAry);
            morphObj -> aryToFile( morphObj -> morphAry, dilateStream );
            morphObj -> prettyPrint( morphObj -> morphAry, prettyPrintStream);

            // step 7
            morphObj -> zero2DAry( morphObj -> morphAry, morphObj -> rowSize, morphObj -> colSize );
            morphObj -> computeErosion( morphObj -> zeroFramedAry, morphObj -> morphAry);
            morphObj -> aryToFile( morphObj -> morphAry, erodeStream );
            morphObj -> prettyPrint( morphObj -> morphAry, prettyPrintStream);

            // step 8
            morphObj -> zero2DAry( morphObj -> morphAry, morphObj -> rowSize, morphObj -> colSize );
            morphObj -> computeOpening( morphObj -> zeroFramedAry, morphObj -> morphAry, morphObj -> tempAry);
            morphObj -> aryToFile( morphObj -> morphAry, closingStream );
            morphObj -> prettyPrint( morphObj -> morphAry, prettyPrintStream );

            // step 9
            morphObj -> zero2DAry( morphObj -> morphAry, morphObj -> rowSize, morphObj -> colSize );
            morphObj -> computeClosing( morphObj -> zeroFramedAry, morphObj -> morphAry, morphObj -> tempAry);
            morphObj -> aryToFile( morphObj -> morphAry, openingStream );
            morphObj -> prettyPrint( morphObj -> morphAry, prettyPrintStream );

            // step 10
            imgStream.close() ;
            structStream.close();
            dilateStream.close() ;
            erodeStream.close();
            closingStream.close();
            openingStream.close();
            delete morphObj ;
    }
    else{ 
        cout << "Error: Input" << endl ; 
    }

    return 0;
}