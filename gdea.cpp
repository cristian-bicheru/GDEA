#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include <cmath>
#include <vector>
#include <symengine/parser.h>
#include <symengine/basic.h>
#include <string>
#include <gmp.h>

using namespace std;

char EQ_MODIFIERS[] = {5,4,3,2,1};

namespace customRandom{
    
    long getTimestamp() {
        // get the amount of seconds since January 1st, 1970
        struct timeval now;
        gettimeofday(&now, NULL);
        return now.tv_sec + now.tv_usec;
    }
    long double getDuration(unsigned int milliseconds){
        // take the unix timestamp for the start and end of the wait time and return the difference
        long double st = getTimestamp();
        usleep(milliseconds);
        long double et = getTimestamp();
        return (long double) (et - st) * 100000000;
    }
    char* getRandom(){
        // get the exact duration to wait 1 millisecond 7 times
        long double d[7];
        for (int i=0; i<7; i++){
            d[i] = getDuration(1);
        }
        // and plug those exact durations into the secret sauce
        mpf_t temp1;
        mpf_init(temp1);
        
        
        mpf_set_str(temp1,to_string(d[2]*d[3]).c_str(),10);
        
        mpf_t temp;
        mpf_init(temp);
        
        mpf_add_ui(temp,temp,10);

        mp_exp_t expo;
        string x = mpf_get_str(NULL,&expo,10,100,temp1);
        mpf_pow_ui(temp,temp,x.length());
        
        mpf_mul(temp,temp,temp1);
        
        // d[0] * d[1]
        mpf_t temp2;
        mpf_init(temp2);
        mpf_set_str(temp2,to_string(d[0]*d[1]).c_str(),10);
        
        mpf_add(temp,temp,temp2);
        
        // (d[4] + d[6]) / d[5]
        mpf_t temp3;
        mpf_init(temp3);
        mpf_set_str(temp3,to_string(d[4]*d[6]).c_str(),10);
        mpf_div_ui(temp3,temp3,d[5]);
        
        mpf_mul(temp,temp,temp3);
        
        mp_exp_t ex;
        
        //return mpf_get_str(NULL,&ex,10,1000,temp);
        return "1";
        /*
        temp = d[2]*d[3]
        out = (d[0]*d[1] + temp * 
            pow(
                10, 
                to_string(temp).length()
            )
        ) * (
            (d[4] + d[6]) / d[5]
        );
        */
        
        //return 1.0;
    }
}

struct KeyPair{
    mpz_t targetArea, blockUCode, offset;

    mpz_t equationModifier[5][2];
};

KeyPair genKeyPair(){
    KeyPair keyPair;  
    
    mpz_init(keyPair.targetArea);
    mpz_init(keyPair.blockUCode);
    mpz_init(keyPair.offset);

    // target area for the line to cover
    mpz_set_str(keyPair.targetArea,customRandom::getRandom(),10);
    // unscrambling code
    mpz_set_str(keyPair.blockUCode,customRandom::getRandom(),10);
    // offset length. Currently disabled due to known bugs
    mpz_add_ui(keyPair.offset,keyPair.offset,1);
    
    static const int equationModifierComplexity = 5;
    // for whatever reason, cristian's original algorithm starts at 1, thus we start at 1
    for (int i=0; i<equationModifierComplexity; i++){
        mpz_t _random;
        
        mpz_init(keyPair.equationModifier[i][0]);
        mpz_init(keyPair.equationModifier[i][1]);
        mpz_init(_random);
        
        mpz_set_str(_random,customRandom::getRandom(),10);
        mpz_mod_ui(_random,_random,5);
        
        mpz_set_str(keyPair.equationModifier[i][0],customRandom::getRandom(),10);
        mpz_add_ui(keyPair.equationModifier[i][1],keyPair.equationModifier[i][1],EQ_MODIFIERS[(int)mpz_get_ui(_random)]);
    }
    
    return keyPair;
}

namespace bigNum{
    struct DivMod{
        vector<unsigned char> divided;
        unsigned int mod;
    };
    void multiply(unsigned char num[], int size, int coefficient){
        
        char carry = 0;
        for(int i=size-1;i>-1;i--){
            short int out = num[i] * coefficient;
            num[i] = out % 256;
            carry = out / 256;
        }
        if (carry != 0){
            cout << "MEMORY OVERFLOW ERROR!!" << endl;
        }
    }
    
    DivMod divmod(unsigned char num[], int size, int denominator){
        
        for(int i=0; i<size; i++){
            cout << (int)num[i] << " ";
        }
        cout << endl;
        
        DivMod dm;
        unsigned int mod = 0;
        for(int i=size-1; i>-1; i--){
            int withCarried = (num[i] + mod*256);
            dm.divided.push_back(withCarried / denominator);
            mod = withCarried % denominator;
        }
        dm.mod = mod;
        for(unsigned char c: dm.divided){
            cout << (int)c << " ";
        }
        return dm;
    }
    
    string addDecimal(string num, mp_exp_t expo){
        bool positive = true;
        if(num.at(0) == '-'){
            positive = false;
            num = num.substr(1,num.length()-2);
        }
        if (expo > 0){
            num = num.substr(0,expo) + "." + num.substr(expo,num.length()-expo);
            if (num.at(num.length()-1) == '.'){
                num += "0";
            }
        } else{
            for(int i=0; i<-expo; i++){
                num = "0" + num;
            }
            num = "0." + num;
        }
        
        if(!positive){
            num = "-" + num;
        }
        return num;
    }
    
    typedef struct{
        long long digits;
        int zeros;
        int power;
        bool positive;
    } HighNum;
    
    HighNum stringToHighNum(string buff){
        HighNum h;
        
        // the first character is either - or a space
        
        h.positive = true;
        if (buff.at(0) == '-'){
            h.positive = false;
        }
        
        // get the unique digits that must be maintained, ignoring the decimal
        string s = buff.at(1) +  buff.substr(3,buff.find((string)"e")-3);
        h.digits = stoll(s);
        // account for the decimal ignored by taking away from the amount of 0s
        h.zeros = stoi(buff.substr(buff.find((string)"+") + 1,buff.length()-buff.find((string)"+") - 1)) - 
            s.length() + 1;
        
        return h;
    }
}

string simplify(string s){
    SymEngine::RCP<const SymEngine::Basic> x = SymEngine::parse(s);
    
    const SymEngine::Basic *test = x.get();
    return test->__str__();
}

string equationListToString(mpf_t equation[6][2]){
    mp_exp_t expo;
    // convert the list of coefficients and terms into a string which is the line's equation
    string s = "";
    for(int i=0; i<5; i++){
        
        string num = mpf_get_str(NULL,&expo,10,100,equation[i][0]);
        string num2 =mpf_get_str(NULL,&expo,10,100,equation[i][1]); 

        // incorperate decimals
        num = bigNum::addDecimal(num,expo);
        num2 = bigNum::addDecimal(num2,expo);

        if (i == 0){
            s += num + "*x + " + num2;
        } else{
            s += " +" + num + "*x**" + num2;
        }

    }
    return s;
}
typedef struct{
    vector<bigNum::HighNum> constants;
    vector<bigNum::HighNum> coefficients;
    vector<bigNum::HighNum> powers;
} Equation;
Equation stringToEquationData(string equation){
    Equation e;
    string buff = "";
    for(int i=0; i<equation.length(); i++){
        if ((i > 0 && equation[i-1] != 'e' && equation[i] == '+') || i == equation.length()-1){
            // put it in the respective vector based on the type of term it is
            if (buff.find((string)"x**") != string::npos){
                // attain the coefficient and exponent of x
                // as integers
                string c = "";
                string p = "";
                bool r = true;
                bool f = true;
                for(int i=0; i<buff.length(); i++){
                    if (r && f && buff.at(i) != '*'){
                        c += buff.at(i);
                    } else if (r && !f && buff.at(i) != '*'){
                        p += buff.at(i);
                    }
                    if (f && buff.at(i) == '*'){
                        r = false;
                        f = false;
                    } else if (!f && buff.at(i) == '*'){
                        r = true;
                    }
                }
                bigNum::HighNum h;
                h.power = stoi(p);
                h.digits = stoi(c);
                h.zeros = 0;
                h.positive = true;
                e.powers.push_back(h);
                
            } else if (buff.find((string)"x") != string::npos){
                string s = buff.substr(0,buff.find((string)"x")-1);
                bigNum::HighNum h;
                if (s.find((string)"e") != string::npos){
                    h = bigNum::stringToHighNum(s);
                } else{
                    h.digits = abs(stoi(s));
                    h.zeros = 0;
                    h.positive = stoi(s) > -1;
                }
                h.power = 1;
                e.coefficients.push_back(h);
            } else{
                bigNum::HighNum h;
                if (buff.find((string)"e") != string::npos){
                    h = bigNum::stringToHighNum(buff);
                } else{
                    h.digits = abs(stoi(buff));
                    h.zeros = 0;
                    h.positive = stoi(buff) > -1;
                }
                h.power = 1;
                e.constants.push_back(h);
            }
            
            // clear the buffer
            buff = "";
        } else{
            buff += equation[i];
        }
    }
    return e;
}
string equationDataToString(Equation e){
    string s = "";
    for(bigNum::HighNum h : e.constants){
        s += to_string(h.digits);
        for(int i=0; i<h.zeros; i++){
            s += "0";
        }
        s += " + ";
    }
    for(bigNum::HighNum h : e.coefficients){
        s += to_string(h.digits);
        for(int i=0; i<h.zeros; i++){
            s += "0";
        }
        s += "*x + ";
    }
    for(bigNum::HighNum h : e.powers){
        s += to_string(h.digits);
        for(int i=0; i<h.zeros; i++){
            s += "0";
        }
        s += "*x**" + to_string(h.power) + " + ";
    }
    
    // remove trailing " + "
    s = s.substr(0,s.length()-3);
    
    return s;
    
}

typedef struct{
    Equation compressorEquation;
    Equation moduloEquation;
    
    vector<unsigned char> compressionFactor;
} CipherEquations;

void encrypt(KeyPair keyPair, unsigned char plainText[], int chunkSize){
    // splits data into many chunks for lower cipher text size and greater efficiency
    int compressor = 65536 * 4;
    
    bigNum::DivMod splitter = bigNum::divmod(plainText,chunkSize,compressor);
    
    // set the max amount of bits for precision
    mpf_set_default_prec(128);
    // encrypting compressor
    
    // SLOPE
    mpf_t slope;
    mpf_init(slope);
    
    mpf_set_z(slope,keyPair.targetArea);
    mpf_mul_ui(slope,slope,2);
    // divisor = pow(compressor-offset,2)
    mpf_t divisor;
    mpf_init(divisor);
    mpf_set_ui(divisor,(unsigned long int)compressor);
    mpf_t temp;
    mpf_init(temp);
    mpf_set_z(temp,keyPair.offset);
    mpf_sub(divisor,divisor,temp);
    mpf_pow_ui(divisor,divisor,(unsigned long int)2);
    
    // set slope to slope/pow(compressor-keyPair.offset,2)
    mpf_div(slope,slope,divisor);
    
    // Y-INT    
    mpf_t b;
    mpf_init(b);
    
    mpf_set_si(b,-mpz_get_si(keyPair.offset));
    mpf_mul(b,slope,b);
    
    mpf_t compressorEquation[5][2];
    
    mpf_init(compressorEquation[0][0]);
    mpf_init(compressorEquation[0][1]);
    
    mpf_set(compressorEquation[0][0], slope);
    mpf_set(compressorEquation[0][1], b);
    
    for(int i=1; i<5; i++){
        mpf_init(compressorEquation[i][0]);
        mpf_init(compressorEquation[i][1]);
        mpf_set_z(compressorEquation[i][0], keyPair.equationModifier[i-1][0]);
        mpf_set_z(compressorEquation[i][1], keyPair.equationModifier[i-1][1]);
    }
    // simplify the equation so it's harder to reverse engineer
    
    string ce = equationListToString(compressorEquation);
    ce = simplify(ce);
    
    // MODULO SLOPE
    mpf_t mslope;
    mpf_init(mslope);
    
    mpf_set_z(mslope,keyPair.targetArea);
    mpf_mul_ui(mslope,mslope,2);
    // divisor = pow(compressor-offset,2)
    mpf_t mdivisor;
    mpf_init(mdivisor);
    mpf_set_ui(mdivisor,(unsigned long int)splitter.mod); // TODO: CHANGE
    mpf_t mtemp;
    mpf_init(mtemp);
    mpf_set_z(mtemp,keyPair.offset);
    mpf_sub(mdivisor,mdivisor,mtemp);
    mpf_pow_ui(mdivisor,mdivisor,(unsigned long int)2);
    
    // set slope to slope/pow(compressor-keyPair.offset,2)
    mpf_div(mslope,mslope,mdivisor);
    
    // MODULO Y-INT    
    mpf_t mb;
    mpf_init(mb);
    
    mpf_set_si(mb,-mpz_get_si(keyPair.offset));
    mpf_mul(mb,mslope,mb);
    
    mpf_t moduloEquation[5][2];
    
    mpf_init(moduloEquation[0][0]);
    mpf_init(moduloEquation[0][1]);
    
    mpf_set(moduloEquation[0][0], mslope);
    mpf_set(moduloEquation[0][1], mb);
    
    for(int i=1; i<5; i++){
        mpf_init(moduloEquation[i][0]);
        mpf_init(moduloEquation[i][1]);
        mpf_set_z(moduloEquation[i][0], keyPair.equationModifier[i-1][0]);
        mpf_set_z(moduloEquation[i][1], keyPair.equationModifier[i-1][1]);
    }
    
    string me = equationListToString(moduloEquation);
    me = simplify(me);
    
    cout << me << endl;
    
    /*
    Equation compe = stringToEquationData(ce);
    Equation mode = stringToEquationData(me);

    vector<bigNum::HighNum> x;
    
    x.push_back(bigNum::HighNum{1,0,1,1});
    
    return (CipherEquations){
        compe,
        mode,
        splitter.divided
    };
    */
}


int main() {
    KeyPair key = genKeyPair();
    
    unsigned char text[] = {1,2,3};
    encrypt(key,text,3);
    //cout << equationDataToString(c.compressorEquation) << endl;
    //cout << equationDataToString(c.moduloEquation) << endl;
    /*
    for(unsigned char v: c.compressionFactor){
        cout << (int)v << " ";
    }
    cout << endl;
    */
    
    return 0;
}