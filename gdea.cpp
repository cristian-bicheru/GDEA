#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include <cmath>
#include <vector>
#include <algorithm>
#include <symengine/parser.h>
#include <symengine/basic.h>
#include <symengine/series.h>
#include <symengine/integer.h>
#include <symengine/sets.h>
#include <symengine/polys/uintpoly_flint.h>
#include <symengine/polys/uintpoly_piranha.h>
#include <symengine/polys/uexprpoly.h>
#include <symengine/polys/uintpoly.h>
#include <symengine/polys/uratpoly.h>
#include <symengine/matrix.h>
#include <symengine/solve.h>
#include <string>
#include <gmp.h>

using namespace std;

char EQ_MODIFIERS[] = {5,4,3,2,1};

void debug(vector<unsigned char> n){
    long long sum = 0;
    for(int i=0;i<n.size();i++){
        cout << (int)n.at(i) << " ";
        sum += n.at(i)*pow(256,i);
    }
    cout << endl;
    cout << "AS INTEGER: " << sum << endl;
}

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
        return "10";
        
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
    
    static const int equationModifierComplexity = 4;
    // for whatever reason, cristian's original algorithm starts at 1, thus we start at 1
    for (int i=0; i<equationModifierComplexity; i++){
        mpz_t _random;
        
        mpz_init(keyPair.equationModifier[i][0]);
        mpz_init(keyPair.equationModifier[i][1]);
        mpz_init(_random);
        
        mpz_set_str(_random,customRandom::getRandom(),10);
        mpz_mod_ui(_random,_random,5);
        
        mpz_set_str(keyPair.equationModifier[i][0],customRandom::getRandom(),10);
        mpz_add_ui(keyPair.equationModifier[i][1],keyPair.equationModifier[i][1],EQ_MODIFIERS[(int)mpz_get_ui(_random)%5]);
    }
    
    return keyPair;
}
namespace byteLogic{
    struct DivMod{
        vector<unsigned char> divided;
        unsigned int mod;
    };
    vector<unsigned char> toChars(int num){
        vector<unsigned char> v;
        int BASE = 256;
        
        //BASE--;
        
        int d  = num / BASE;
        int m = num % BASE;
        while(d != 0){
            v.push_back(m);
            m = d % BASE;
            d = d / BASE;
        }
        v.push_back(m);
        
        return v;
    }
    void multiply(vector<unsigned char> *num, int coefficient){
        
        unsigned int carry = 0;
        for(int i=0;i<num->size();i++){
            unsigned int out = carry + num->at(i) * coefficient;
            num->at(i) = out % 256;
            carry = out / 256;
        }
        if (carry != 0){
            vector<unsigned char> v;
            v = toChars(carry);
            for(unsigned char c: v){
                num->push_back(c);
            }
        }
    }
    
    void add(vector<unsigned char> *num, long int a){
        vector<unsigned char> broken = toChars(a);
        
        unsigned char carry = 0;
        for(int i=0;i<num->size();i++){
            char add = 0;
            if (i <= broken.size()-1){
                add = broken.at(i);
            }
            num->at(i) = (num->at(i) + carry + add) % 256;
            carry = (num->at(i) + carry + add) / 256;
        }
        if (carry != 0){
            num->insert(num->begin(),1,carry);
        }
    }
    
    DivMod divmod(vector<unsigned char> num, int denominator){
        DivMod dm;
        int mod = 0;
        for(int i=num.size()-1; i>-1; i--){
            dm.divided.insert(dm.divided.begin(),1,(num.at(i)+mod*256)/denominator);
            mod = (num.at(i)+mod*256)%denominator;
        }
        dm.mod = mod;
        return dm;
    }
}
namespace convert{
    string removeDecimal(string s){
        int x = s.find(".");
        return s.substr(0,x);
    }
    int getE(string s){
        int i = s.find('e');
        return stoi(s.substr(i+1,s.length()-i));
    }
    string addE(string num, int expo){
        // split it up into the sign, first digit and the rest of the digits
        bool positive = true;
        if(num.at(0) == '-'){
            positive = false;
            num = num.substr(1,num.length()-1);
        }
        char first = num.at(0);
        if(num.length() < 2){
            num = "";
        } else{
            num = num.substr(2,num.length()-2);
        }
        
        // add zeros / move decimal
        string out = "";
        out += first;
        if (expo > 0){
            // starts at 10^0 even if expo=1, so add 1
            expo++;
            // move the decimal right
            int count = 0;
            for(int i=0; i<num.length(); i++){
                if(i == expo){
                    out += ".";
                    
                }
                out += num.at(i);
                count++;
            }
            // add in extra zeros
            for(int i=0; i<(expo+1)-out.length();i++){
                out += "0";
            }
        } else{
            expo++;
            out += num;
            for(int i=0; i<-expo; i++){
                out = "0" + out;
            }
            out = "0." + out;
        }
        if(!positive) out = '-' + out;
        return out;
    }
    string addDecimal(string num, mp_exp_t expo){
        bool positive = true;
        if(num.length() == 0){
            cout << "ERROR! addDecimal RECIEVED NOTHING AS NUM" << endl;
        }
        if(num.at(0) == '-'){
            positive = false;
            num = num.substr(1,num.length()-1);
        }
        string out = "";
        if(expo > 0){
            int count = 0;
            for(int i=0; i<min((int)expo,(int)num.length()); i++){
                out += num.at(i);
                count++;
            }
            // if there are still decimals left
            int x = out.length();
            if(x < num.length()){
                out += ".";
            }
            for(int i=x; i<num.length();i++){
                out += num.at(i);
                count++;
            }
            // if extra zeros are required
            if(num.length() < expo){
                for(int i=0;i<expo-num.length();i++){
                    out += "0";
                }
            }
        } else if(expo < 0) {
            out = num;
            // simply adding zeros to the left side of the digits
            for(int i=0; i<-expo; i++){
                out = "0" + out;
            }
            out = "0." + out;
        } else{
            out = "0." + num;
        }
        if (!positive){
            out = '-' + out;
        }
        return out;
    }
    string equationListToString(mpf_t equation[][2],int size, bool allpowers=false){
        mp_exp_t expo;
        // convert the list of coefficients and terms into a string which is the line's equation
        string s = "";
        for(int i=0; i<size; i++){
            
            string num = mpf_get_str(NULL,&expo,10,100,equation[i][0]);
            num = addDecimal(num,expo);

            string num2 = mpf_get_str(NULL,&expo,10,100,equation[i][1]); 
            num2 = addDecimal(num2,expo);
    
            if (i == 0 && !allpowers){
                s += num + "*x ";
                if(num2.find("-") != string::npos){
                    s += num2;
                } else{
                    s += "+ " + num2;
                }
            } else{
                s += " +" + num + "*x**" + removeDecimal(num2);
            }
    
        }
        return s;
    }
    string equationListToString(mpz_t equation[][2],int size, bool allpowers=false){
        // convert the list of coefficients and terms into a string which is the line's equation
        string s = "";
        for(int i=0; i<size; i++){
            
            string num = mpz_get_str(NULL,10,equation[i][0]);
            string num2 = mpz_get_str(NULL,10,equation[i][1]); 
            
            if (i == 0 && !allpowers){
                s += num + "*x ";
                if(num2.find("-") != string::npos){
                    s += num2;
                } else{
                    s += "+ " + num2;
                }
            } else{
                s += " +" + num + "*x**" + removeDecimal(num2);
            }
    
        }
        return s;
    }
    
    typedef struct{
        mpf_t digits;
        mpf_t power;
    } HighNum;
    
    typedef struct{
        vector<HighNum> constants;
        vector<HighNum> coefficients;
        vector<HighNum> powers;
    } Equation;
    Equation stringToEquationData(string equation){
        Equation e;
        string buff = "";
        string other = "";
        // first fix the terms (stored as e-11 or e+2, etc.)
        string ns = "";
        bool bad = false;
        bool badbad = false;
        for(int i=0; i<equation.length(); i++){
            
            char c = equation.at(i);
            
            if(c == '*') badbad = true;
            if(c == '+'){
                ns += c;
            } else if(c == 'e'){
                bad = true;
                buff += c;
            } else if((c == ' ' || i == equation.length()-1) && bad){
                if (!badbad){
                    buff += c;
                } else{
                    other += c;
                }
                string x = buff.substr(0,buff.find('e'));
                ns += convert::addE(x,getE(buff));
                ns += other + " ";
                
                buff = "";
                other = "";
                bad = false;
                badbad = false;
            } else if (c == ' ' || i == equation.length()-1){
                if (!badbad){
                    buff += c;
                } else{
                    other += c;
                }
                ns += buff + other;
                
                buff = "";
                other = "";
                bad = false;
                badbad = false;
            } else if (badbad){
                other += c;
            } else{
                buff += c;
            }
        }
        // STRING IS NOW THE FIXED STRING
        equation = ns;
        
        // convert to more compact data
        for(int i=0; i<equation.length(); i++){
            if ((i > 0 && (equation.at(i) == '+' || equation.at(i) == '-')) || i == equation.length()-1){
                if(equation.at(i) == '-') buff += equation.at(i);
                // put it in the respective vector based on the type of term it is
                if (buff.find((string)"x**") != string::npos){
                    buff += equation.at(i);
                    // attain the coefficient and exponent of x
                    // as integers
                    string c = "";
                    string p = "";
                    bool r = true;
                    bool f = true;
                    for(int j=0; j<buff.length(); j++){
                        if (r && f && buff.at(j) != '*'){
                            c += buff.at(j);
                        } else if (r && !f && buff.at(j) != '*'){
                            p += buff.at(j);
                        }
                        if (f && buff.at(j) == '*'){
                            r = false;
                            f = false;
                        } else if (!f && buff.at(j) == '*'){
                            r = true;
                        }
                    }
                    HighNum h;
                    mpf_init(h.digits);
                    mpf_init(h.power);
                    mpf_set_str(h.digits,c.c_str(),10);
                    mpf_set_str(h.power,p.c_str(),10);
                    e.powers.push_back(h);
                    
                } else if (buff.find("x") != string::npos){
                    string s = buff.substr(0,buff.find((string)"x")-1);
                    HighNum h;
                    mpf_init(h.digits);
                    mpf_init(h.power);
                    mpf_set_str(h.digits,s.c_str(),10);
                    mpf_set_ui(h.power,(unsigned long int)1);
                    e.coefficients.push_back(h);
                } else{
                    HighNum h;
                    mpf_init(h.digits);
                    mpf_init(h.power);
                    mpf_set_str(h.digits,buff.c_str(),10);
                    mpf_set_ui(h.power,(unsigned long int)1);
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
        // TODO: known bug with 4 at this next for loop
        for(HighNum h : e.constants){
            mp_exp_t expo;
            string x = mpf_get_str(NULL,&expo,10,100,h.digits);
            s += addDecimal(x,expo);
            s += " + ";
        }
        for(HighNum h : e.coefficients){
            mp_exp_t expo;
            string x = mpf_get_str(NULL,&expo,10,100,h.digits);

            s += addDecimal(x,expo);
            s += "*x + ";
        }

        for(HighNum h : e.powers){
            mp_exp_t expo;
            string x = mpf_get_str(NULL,&expo,10,100,h.digits);

            s += addDecimal(x,expo);
            x = mpf_get_str(NULL,&expo,10,100,h.power);
            
            // IGNORE THE FLOAT BECAUSE IT FUCKS SYMENGINE UP
            s += "*x**" + x + " + ";
        }
        // remove the trailing " + " left by the for loops appending to s
        if (e.constants.size() > 0 || e.coefficients.size() > 0 || e.powers.size() > 0){
            s = s.substr(0,s.length()-2);
        }
        
        return s;
    }
}
namespace engine{
    vector<string> solve(string s){
        SymEngine::RCP<const SymEngine::Basic> e = SymEngine::parse(s);
        SymEngine::RCP<const SymEngine::Symbol> x  = SymEngine::make_rcp<SymEngine::Symbol>("x");
        
        SymEngine::RCP<const SymEngine::Set> solved = SymEngine::solve(e,x);
        const SymEngine::Set *test = solved.get();
        
        string out = test->__str__();
        vector<string> v;
        
        string buff = "";
        out = out.substr(1,out.length()-2);
        for(int i=0; i<out.length(); i++){
            if (out.at(i) == ',' || i == out.length()-1){
                
                if (i == out.length()-1) buff += out.at(i);
                v.push_back(buff);
                
                buff = "";
            } else{
                buff += out.at(i);
            }
        }
        
        return v;
    }
    string simplify(string s){
        SymEngine::RCP<const SymEngine::Basic> e = SymEngine::parse(s);
        
        const SymEngine::Basic *test = e.get();
        string x = test->__str__();
        return x;
    }
}

namespace core{
    typedef struct {
        mpf_t equation[5][2];
    } EquationSet;
    /**
     * Do many calculations with the randoms generated to encode it.
     */
    EquationSet secretSauce(KeyPair keyPair, byteLogic::DivMod splitter, unsigned long int spice){
        
        // SLOPE
        mpf_t slope;
        mpf_init(slope);
        
        mpf_set_z(slope,keyPair.targetArea);
        mpf_mul_ui(slope,slope,2);
        // divisor = pow(compressor-offset,2)
        mpf_t divisor;
        mpf_init(divisor);
        mpf_set_ui(divisor,(unsigned long int)spice);
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
        
        // define the output
        
        EquationSet output;
        
        
        mpf_init(output.equation[0][0]);
        mpf_init(output.equation[0][1]);
        
        mpf_set(output.equation[0][0], slope);
        mpf_set(output.equation[0][1], b);
        for(int i=1; i<5; i++){
            //mpz_out_str(stdout,10,keyPair.equationModifier[i-1][0]);
            //mpz_out_str(stdout,10,keyPair.equationModifier[i-1][1]);

            mpf_init(output.equation[i][0]);
            mpf_init(output.equation[i][1]);

            mpf_set_z(output.equation[i][0], keyPair.equationModifier[i-1][0]);
            mpf_set_z(output.equation[i][1], keyPair.equationModifier[i-1][1]);
        }
        
        return output;
    }

    /**
     * Solves an symbolic integral where the symbol is x.
     */
    string integrate(string s){
        vector<string> terms;
        string buff = "";
        for(int i=0; i<s.length(); i++){
            if (i == s.length()-1){
                buff += s.at(i);
            }
            if(s.at(i) == '+' || i == s.length()-1){
                terms.push_back(buff);
                buff = "";
            } else{
                buff += s.at(i);
            }
        }
        string out = "";
        for(int i=0; i<terms.size(); i++){
            string temp;
            if (i == 0){
                temp = terms.at(terms.size() - 1);
            } else{
                temp = terms.at(i-1);
            }
            if(temp.find("**") != string::npos){
                string s = temp.substr(
                    temp.find("**") + 2,
                    temp.length()-temp.find("**")
                );
                int power = stoi(s) + 1;
                string num = temp.substr(0,temp.find("**"));
                
                if (num.find("x") != string::npos){
                    if (num.length() <= 2){
                        num = "1";
                    } else{
                        num = num.substr(0,num.length()-2);
                    }
                }
                mpf_t _num;
                mpf_init(_num);
                mpf_set_str(_num,num.c_str(),10);
                mpf_t _power;
                mpf_init(_power);
                mpf_set_si(_power,power);
                
                mpf_div(_num,_num,_power);
                mp_exp_t expo;
                string frac = mpf_get_str(NULL,&expo,10,100,_num);
                frac = convert::addDecimal(frac,expo);
                temp = frac + "*x";
                temp = temp + "**" + to_string(power);
            } else if (temp.find("x") != string::npos){
                mpf_t _temp;
                mpf_init(_temp);
                mpf_set_str(_temp,temp.substr(0,temp.length()-2).c_str(),10);
                mpf_div_ui(_temp,_temp,2);
                mp_exp_t expo;
                string x = mpf_get_str(NULL,&expo,10,100,_temp);
                x = convert::addDecimal(x,expo);
                temp = x + "*x**2";
            } else{
                temp = temp + "*x";
            }
            if(i == 0){
                out = temp;
            } else{
                out += "+" + temp;
            }
        }
        string newout = "";
        for(int i=0;i<out.length();i++){
            if (out.at(i) != ' '){
                newout += out.at(i);
            }
        }
        return newout;
    }
}

typedef struct{
    convert::Equation compressorEquation;
    convert::Equation moduloEquation;
    
    vector<unsigned char> compressionFactor;
} CipherEquations;

CipherEquations encrypt(KeyPair keyPair, unsigned char plainText[], int chunkSize){
    vector<unsigned char> plainVector;
    for(int i=0; i<chunkSize; i++){
        plainVector.push_back(plainText[i]);
    }
    // splits data into many chunks for lower cipher text size and greater efficiency
    int compressor = 65536 * 4;
    byteLogic::multiply(&plainVector,(int)mpz_get_ui(keyPair.blockUCode));
    byteLogic::DivMod splitter = byteLogic::divmod(plainVector,compressor);
    
    // set the max amount of bits for precision
    mpf_set_default_prec(128);
    
    // first encode the compressor
    core::EquationSet packagedEquation = core::secretSauce(keyPair, splitter, compressor);

    string compressorEquation = engine::simplify(
        convert::equationListToString(
            packagedEquation.equation,
            5
        )
    );
    convert::Equation compressorData = convert::stringToEquationData(compressorEquation);

    // next the modulo
    core::EquationSet packagedModEquation = core::secretSauce(keyPair, splitter, splitter.mod);
    string moduloEquation = engine::simplify(
        convert::equationListToString(
            packagedModEquation.equation,
            5
        )
    );
    convert::Equation moduloData = convert::stringToEquationData(moduloEquation);
    
    return (CipherEquations){
        compressorData,
        moduloData,
        splitter.divided
    };
    
}

vector<unsigned char> decrypt(KeyPair key, CipherEquations cipher){
    
    // DECRYPT COMPRESSOR
    string compressorEquation = engine::simplify(
        convert::equationDataToString(cipher.compressorEquation)
    );
    compressorEquation += " - (" + convert::equationListToString(key.equationModifier,4,true) + ")";
    
    string integrated = engine::simplify(core::integrate(engine::simplify(compressorEquation)));
    
    string replaced = "";
    for(int i=0; i<integrated.length();i++){
        char c = integrated.at(i);
        if(c == 'x'){
            replaced += mpz_get_str(NULL,10,key.offset);
        } else{
            replaced += c;
        }
    }
    compressorEquation = "(" + integrated + ") - (" + engine::simplify(replaced) + ") - " + mpz_get_str(NULL,10,key.targetArea);
    // find compressor (the positive value)
    long double compressor;
    for(string s: engine::solve(engine::simplify(compressorEquation))){
        long double temp = stod(s);
        if(temp > 0){
            compressor = temp;
            break;
        }
    }
    
    // DECRYPT MOD
    string moduloEquation = engine::simplify(
        convert::equationDataToString(cipher.moduloEquation)
    );
    moduloEquation += " - (" + convert::equationListToString(key.equationModifier,4,true) + ")";
    
    string integratedMod = engine::simplify(core::integrate(engine::simplify(moduloEquation)));
    
    replaced = "";
    for(int i=0; i<integratedMod.length();i++){
        char c = integratedMod.at(i);
        if(c == 'x'){
            replaced += mpz_get_str(NULL,10,key.offset);
        } else{
            replaced += c;
        }
    }
    moduloEquation = "(" + integratedMod + ") - (" + engine::simplify(replaced) + ") - " + mpz_get_str(NULL,10,key.targetArea);
    // find compressor (the positive value)
    long double modulo;
    for(string s: engine::solve(engine::simplify(moduloEquation))){
        long double temp = stod(s);
        if(temp > 0){
            modulo = temp;
            break;
        }
    }
    // decrypt the compressionFactor (the actual data)
    byteLogic::multiply(&cipher.compressionFactor,compressor);
    byteLogic::add(&cipher.compressionFactor,modulo);
    byteLogic::DivMod out = byteLogic::divmod(cipher.compressionFactor,mpz_get_ui(key.blockUCode));
    
    return out.divided;
}
int main(){
    KeyPair key = genKeyPair();
    
    unsigned char text[] = {1,2,3};
    CipherEquations c = encrypt(key, text, 3);
    vector<unsigned char> v = decrypt(key,c);
    
    for(unsigned char x: v){
        cout << (int)x << endl;
    }
    return 0;
}
