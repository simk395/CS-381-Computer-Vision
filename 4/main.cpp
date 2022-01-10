#include <iostream>
#include <fstream>
#include <cmath>
using namespace std;

class BiMean{
    public:
        int numRows,
            numCols,
            minVal,
            maxVal,
            maxHeight,
            maxGVal,
            offset,
            dividePt,
            *histAry,
            *gaussAry,
            **histGraph,
            **gaussGraph,
            **gapGraph;
    
    public:
        BiMean( ifstream &input ){
            read_header( input );
            this -> offset = (int)( this -> maxVal - this -> minVal ) / 10;
            this -> dividePt = this -> offset;
            
            this -> histAry = new int[ this -> maxVal + 1 ];
            this -> gaussAry = new int[ this -> maxVal + 1 ];
            this -> histGraph = new int*[ this -> maxVal + 1 ];
            this -> gaussGraph = new int*[ this -> maxVal + 1 ];
            this -> gapGraph = new int*[ this -> maxVal + 1 ];

            findMaxHeight(input);
            input.clear();
            input.seekg(0);
            ignore_header(input);
            
            for( int i = 0; i < this -> maxVal + 1; i++){
                this -> histGraph[i] = new int[ this -> maxHeight + 1 ];
                this -> gaussGraph[i] = new int[ this -> maxHeight + 1 ];
                this -> gapGraph[i] = new int[ this -> maxHeight + 1 ];
            }
            
            set1DZero(this -> histAry);
            set1DZero(this -> gaussAry);
            set2DZero(this -> histGraph);
            set2DZero(this -> gapGraph);
            set2DZero(this -> gapGraph);
        }

        int biMeanGauss( int dividePt, ofstream &output){
            double sum1, sum2, total, minSumDiff;
            int bestThr = dividePt;
            minSumDiff = 999999.0;
            while( dividePt < (this->maxVal - this->offset) ){
                // step 1
                set1DZero(this -> gaussAry);
                set2DZero(this -> gaussGraph);
                set2DZero(this -> gapGraph);

                // step 2
                sum1 = fitGauss( 0, dividePt, this -> gaussAry, this -> gaussGraph, output );
                // step 3
                sum2 = fitGauss( dividePt, this -> maxVal, this -> gaussAry, this -> gaussGraph, output);
                // step 4
                total = sum1 + sum2;
                output << "Sum of left fitting: " << sum1 << " Sum right fitting: " << sum2 << " Total: " << total << endl;
                // step 5
                if(total < minSumDiff){
                    minSumDiff = total;
                    bestThr = dividePt;
                } 
                output << "Divide Point: " << dividePt << " Minimum Sum Difference: " << minSumDiff << " Best Threshold: " << bestThr << endl;
                dividePt++;
                
                prettyPrint(this -> gaussGraph, output);
                plotGaps(this -> histAry, this -> gaussAry, this -> gapGraph);
                output << "Gap Graph with divide point at: " << dividePt - 1 << endl;
                prettyPrint(this -> gapGraph, output);
            }
            
            return bestThr;
        }

        void bestFitPlot(int thr, ofstream &output){
            double sum1, sum2;

            set1DZero( this -> gaussAry);
            set2DZero( this -> gaussGraph);
            set2DZero( this -> gapGraph);

            output << "Fitting through bestFitPlot method: " << endl;
            sum1 = fitGauss( 0, thr, this -> gaussAry, this -> gaussGraph, output );
            sum2 = fitGauss( thr, this -> maxVal, this -> gaussAry, this -> gaussGraph, output );
            plotGaps( this -> histAry, this -> gaussAry, this -> gapGraph );
        }

        double computeMean( int leftIndex, int rightIndex, int height ){
            height = 0;
            int sum = 0;
            int numPixels = 0;
            int index = leftIndex;

            while(index < rightIndex){
                sum += ( this -> histAry[index] * index );
                numPixels += this -> histAry[index];
                if( this -> histAry[index] > height ) maxHeight = this -> histAry[index];
                index++;
            }
            
            return (double)sum / (double)numPixels;
        }

        double computeVar( int leftIndex, int rightIndex, double mean){
            double sum = 0.0;
            int numPixels = 0;
            int index = leftIndex;

            while( index < rightIndex ){
                sum += (double)this -> histAry[index] * pow( ((double)index - mean), 2);
                numPixels += this -> histAry[index];
                // sum += pow( ((double)index - mean), 2);
                // numPixels++;
                index++;
            }

            return (double) sum / (double) numPixels;
            
        } 

        void findMaxHeight(ifstream &input){
            int current = 0;
            int max = 0;
            for(int i = 0; i < this -> maxVal + 1; i++){
                input >> i >> current;
                if( current > max ) max = current;
            }
            this -> maxHeight = max;
        }

        double fitGauss( int leftIndex, int rightIndex, int *ary, int **graph, ofstream &output){
            double mean, var, sum, gVal, maxGVal;
            sum = 0.0;
            mean = computeMean(leftIndex, rightIndex, this -> maxHeight);
            var = computeVar(leftIndex, rightIndex, mean);
            output << "Left Index: " << leftIndex << " Right Index: " << rightIndex << " Mean: " << mean << " Variance: " << var << endl;
            
            int index = leftIndex;
            while( index <= rightIndex ){
                gVal = modifiedGauss(index, mean, var, this -> maxHeight);
                // sum += abs(gVal - (double)this -> histAry[index]);
                sum += abs( (double)this -> histAry[index] - gVal );
                ary[index] = (int) gVal;
                graph[index][(int) gVal] = 1;
                index++;
            }
            
            return sum;
        }

        void loadHist(int *ary, ifstream &input){
            for(int i = 0; i < this -> maxVal + 1; i++){
                input >> i >> this -> histAry[i];
            }   
        }

        void ignore_header( ifstream &input ){
            int i;
            input >> i >> i >> i >> i;
        }

        double modifiedGauss( int index, double mean, double var, int height){
            return (double)(height * exp(- ( pow( (index-mean), 2) / (2 * var)) ));
        }

        void plotGaps( int *ary, int *ary2, int **graph){
            int index = this -> minVal;

            while( index < this -> maxVal ){
                int first = min( ary[index], ary2[index] );
                int last = max( ary[index], ary2[index] );
                    while(first < last){
                        graph[index][first] = 1;
                        first++;
                    }
                index++;
            }
        }

        void plotHistGraph( int **ary ){
            for( int i = 0; i < this -> maxVal + 1; i++){
                for( int j = 0; j < this -> histAry[i]; j++){
                    ary[i][j] = 1;
                    
                }
            }
        }

        void prettyPrint( int **ary, ofstream &output ){
            for( int i = 0; i < this -> maxVal + 1; i++){
                for( int j = 0; j < this -> maxHeight + 1; j++){
                    if( ary[i][j] <= 0 ) output << " ";
                    else output << ".";
                }
                output << endl;
            }
        }

        void read_header( ifstream &input ){
            input >> this -> numRows >> this -> numCols >> this -> minVal >> this -> maxVal ;
        }

        void set1DZero(int *ary){
            for(int i = 0; i < this -> maxVal + 1; i++){
                ary[i] = 0;
            }
        }

        void set2DZero(int **ary){
            for(int i = 0; i < this -> maxVal + 1; i++){
                for(int j = 0; j < this -> maxHeight + 1; j++){
                    ary[i][j] = 0;
                }
            }
        }   
};

int main( int argc, const char *argv[] ){

    string inFile = argv[1];
    string outFile1 = argv[2];
    string outFile2 = argv[3];

    ifstream input; 
    input.open( inFile ) ;

    ofstream output1, output2, output3; 
    output1.open(outFile1);
    output2.open(outFile2);

    if(input.is_open()){
        if(output1.is_open() && output2.is_open()){

            
            BiMean *biMeanObj = new BiMean( input );

            biMeanObj -> loadHist( biMeanObj -> histAry, input );
            biMeanObj -> plotHistGraph( biMeanObj -> histGraph );
            output1 << "2D Display of Histogram from given input: " << endl;
            biMeanObj -> prettyPrint( biMeanObj -> histGraph, output1 );

            int bestThrVal = biMeanObj -> biMeanGauss( biMeanObj -> dividePt, output2 );
            output1 << "Best Threshold Value: " << bestThrVal << endl;

            biMeanObj -> bestFitPlot(bestThrVal, output2);

            output1 << "Best fitted plotting: " << endl;
            biMeanObj -> prettyPrint(biMeanObj -> gaussGraph, output1);

            output1 << "Gap graph with best fit plotted: " << endl;
            biMeanObj -> prettyPrint(biMeanObj -> gapGraph, output1);
        
            input.close();
            output1.close();
            output2.close();
        }
    }
    

    exit(1);
}