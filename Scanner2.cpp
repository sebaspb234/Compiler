
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

vector<string> keywords = { "and", "as", "assert", "async", "await", "bool", "break", 
"class","continue", "def", "del", "elif", "else", "except", "False", "finally","float", 
"for", "from", "global", "if", "import", "in", "int", "is","lambda", "None", "nonlocal", 
"not", "or", "pass", "raise", "return","str", "True", "try", "while", "with", "yield" };

vector<string> operadores = { "+", "-", "*", "/","//", "%", "<", ">", "<=", 
">=", "==", "!=", "=", ",", ":", ".", "->" };

vector<string> delimitadores = { "(", ")", "[", "]" };

struct Token
{
    string valor, tipo, error;
    int fila, columna;
    Token(string valor_, string tipo_, int fila_, int columna_, string error_ = "")
    {
        valor = valor_;
        tipo = tipo_;
        fila = fila_;
        columna = columna_;
        error = error_;
    }
};

struct Scanner
{
    vector<Token> tokens;
    vector<string> lineas;
    vector<Token> errores;
    string nombreArchivo;
    int fila, columna;

    Scanner(string nombreArchivo_)
    {
        nombreArchivo = nombreArchivo_;
        fila = columna = 0;
    }
    void leerArchivo();
    void printLinea();
    void escaner();
    char getchar();
    char peekchar();
    void imprimirTokens();
    void imprimirErrores();
    //void verificarEspacios(); // se utiliza para evitar colocar newline luego de que ocurra un error y no se detecte un token. PREGUNTAR, creo que esta bien asi
};

char Scanner::getchar()
{
    char c = lineas[fila][columna];
    columna++;
    return c;
}

char Scanner::peekchar()
{
    char c = lineas[fila][columna];
    return c;
}

void Scanner::escaner()
{
    string token = ""; // va a guardar el valor del token
    
    // voy a leer linea por linea del vector de strings lineas
    for (; fila < lineas.size(); fila++)
    {
        bool primera_tabulacion = false; // se utiliza para solo reconocer tabs al inicio de la linea
        bool primer_espacio = false;
        cout << "[";
        cout << fila << "";
        for (columna = 0; columna < lineas[fila].size();)
        {

            char caracter = getchar();//lineas[fila][columna]; // primer caracter de la linea
            int columna_encontrado; // guarda la columna en la que se encuentra al token por primera vez
            char next_caracter; // siguiente caracter
            //bool primera_tabulacion = false; // se utiliza para solo reconocer tabs al inicio de la linea
            
            if (caracter == 9 || caracter >= 32 && caracter <= 126) // no acepta caracteres que no esten entre 32 y 126, solo acepta a la tabulacion
            {
                //KEYWORDS
                if (isalpha(caracter))
                {
                    int columna_encontrado = columna;// columna_encontrado es donde se encontró el token por primera vez
                    token += caracter; // lo añado al token
                    next_caracter = peekchar(); // agarro el siguiente caracter
                    while (isalpha(next_caracter) || isdigit(next_caracter) || next_caracter == '_')
                    {
                        token += getchar();
                        next_caracter = peekchar();
                    }
                    // al terminar
                    if (find(keywords.begin(), keywords.end(), token) != keywords.end()) // si el token está en las keywords
                    {
                        tokens.push_back(Token(token, "KEYWORD", fila + 1, columna_encontrado));
                    }
                    else // si no es keyword, es identificador
                    {
                        tokens.push_back(Token(token, "ID", fila + 1, columna_encontrado));
                    }
                    token = ""; // reseteo el token

                }

                // NUMEROS
                else if (isdigit(caracter))
                {
                    columna_encontrado = columna;
                    token += caracter; // se añade el numero al token
                    next_caracter = peekchar();
                    while (isdigit(next_caracter) || next_caracter == '.')
                    {
                        token += getchar();
                        next_caracter = peekchar();
                    }
                    if (token.size() > 1 && token[0] == '0') // si es un numero que tiene más de un digito y empieza en 0, no es válido
                    {
                        errores.push_back(Token(token, "ERROR", fila + 1, columna_encontrado, "NUMERO INVALIDO"));
                    }
                    else if (token.find('.') != -1) // si encuentra un punto en el numero es float
                    {
                        tokens.push_back(Token(token, "FLOAT", fila + 1, columna_encontrado));
                    }
                    else // es un numero entero normal
                    {
                        long long max_entero = stoll(token); // verifica el limite de un entero
                        if (max_entero > INT_MAX)
                        {
                            errores.push_back(Token(token, "ERROR", fila + 1, columna_encontrado, "ENTERO SOBREPASA EL LIMITE"));
                        }
                        else // acepta el entero
                        {
                            tokens.push_back(Token(token, "INT", fila + 1, columna_encontrado));
                        }
                    }
                    token = ""; // reseteo el token
                }

                //OPERADORES
                else if (find(operadores.begin(), operadores.end(), string(1, caracter)) != operadores.end()) // si el caracter es un operador
                {

                    columna_encontrado = columna;
                    token += caracter;
                    next_caracter = peekchar(); // evaluo el siguiente caracter
                    if ((caracter == '+' && next_caracter == '+') || (caracter == '-' && next_caracter == '-') || (caracter == '-' && next_caracter == '>') || (caracter == '=' && next_caracter == '='))
                    {
                        token += getchar();
                    }
                    tokens.push_back(Token(token, "OP", fila + 1, columna_encontrado)); // añade operador
                    token = "";

                }

                // DELIMITADORES
                else if (find(delimitadores.begin(), delimitadores.end(), string(1, caracter)) != delimitadores.end())
                {
                    columna_encontrado = columna;
                    token += caracter;
                    if (caracter == '(') tokens.push_back(Token(token, "OPEN_PAR", fila + 1, columna_encontrado));
                    if (caracter == ')') tokens.push_back(Token(token, "CLO_PAR", fila + 1, columna_encontrado));
                    if (caracter == '[') tokens.push_back(Token(token, "OPEN_COR", fila + 1, columna_encontrado));
                    if (caracter == ']') tokens.push_back(Token(token, "CLO_COR", fila + 1, columna_encontrado));
                    token = "";
                }

                // STRINGS
                else if (caracter == '"')
                {
                    token += caracter;
                    next_caracter = peekchar();
                    columna_encontrado = columna;
                    string error_string = "";
                    bool error = false; // indicador si hubo error o no
                    bool segunda_comilla = false; // verifica si la cadena se cierra luego de encontrar \"
                    while (next_caracter != '"' && columna < lineas[fila].size())
                    {
                        
                        token += getchar();
                        next_caracter = peekchar();
                    }
                    
                    ////
                    // caso \"
                    int pos = lineas[fila].find('\\');
                    if (pos != -1) // sí encontró un \\a
                    {
                        if (lineas[fila][pos + 1] != '"') // error
                        {
                            // asigno el valor invalido a error_string
                            error_string = token.substr(pos,token.size());
                            errores.push_back(Token(error_string, "ERROR", fila + 1, columna, "STRING INVALIDO"));
                            error = true;
                        }
                        else // si existe un \", verificar que hay una comilla que cierra
                        {// seguimos recorriendo hasta que encuentre otra comilla
                            while (next_caracter != '"' && columna < lineas[fila].size())
                            {

                                token += getchar();
                                next_caracter = peekchar();
                                if (next_caracter == '"')
                                {
                                    segunda_comilla = true;// encontró comilla de cierre
                                    error = false;
                                }
                            }
                        }
                    }
                    token += getchar();
                    if (!error) // si no hubo error, y si se cierra correctamente, entonces sí se debe crear el token
                    {
                        tokens.push_back(Token(token, "STRING", fila + 1, columna_encontrado));
                    }
                    ///
                    //token += getchar();
                    token = "";
                }

                //ESPACIOS REVISAR!!!!!!!!!!!!
                else if (primer_espacio == false && caracter == ' ')
                {
                    string::iterator it;
                    for (it = lineas[fila].begin(); it != lineas[fila].end(); ++it)
                    {// cuento la cantidad de tabs
                        primer_espacio = true; // luego de entrar por primera vez, ya no vuelve a entrar
                        if ((*it) == ' ')
                        {
                            tokens.push_back(Token(" ", "ESPACIO", fila + 1, columna));
                        }
                        else
                        {
                            break;// si encuentra otro caracter que no es tab, se sale del for
                        }
                    }
                }
                
                // TABULACIONES, CARACTER == 9
                else if (primera_tabulacion == false && caracter == 9)
                {
                // SOLO DEBE RECONOCER LOS TABS DEL INICIO
                //columna_encontrado = columna;
                    string::iterator it;
                    for (it = lineas[fila].begin(); it != lineas[fila].end(); ++it)
                    {// cuento la cantidad de tabs
                        primera_tabulacion = true; // luego de entrar por primera vez, ya no vuelve a entrar
                        if ((*it) == 9)
                        {
                            tokens.push_back(Token("TAB", "TAB", fila + 1, columna));
                        }
                        else
                        {
                            break;// si encuentra otro caracter que no es tab, se sale del for
                        }
                    }

                }

                else
                {
                    errores.push_back(Token(string(1,caracter), "ERROR", fila + 1, columna, "TOKEN NO RECONOCIDO"));
                    columna++; // avanza columna, no estoy seguro
                }

                //cout << caracter;
                if (columna == lineas[fila].size()) //cout << "salto";
                {
                    tokens.push_back(Token("", "NEWLINE", fila + 1, columna + 1)); // REVISAR COLUMNA QUE SE IMPRIME EN TOKEN, AUMENTAR +1
                }
            }
            else
            {
                errores.push_back(Token(string(1,caracter), "ERROR", fila + 1, columna, "CARACTER INVALIDO"));
            }
            

            

        }cout << "]" << endl;
    }
}

void Scanner::leerArchivo() {
    
    ifstream archivo(nombreArchivo);

    if (!archivo) {
        cerr << "No se pudo abrir el archivo " << nombreArchivo << endl;
        return;
    }

    string linea;
    while (getline(archivo, linea)) {
        
        int cant_espacios = 0; // cantidad de espacios se comparará con el size para saber si todos son espacios
        int cant_tabs = 0; // cantidad de tabs
        int comentario = linea.find('#'); // busco un comentario
        int suma_chars = 0; // variable de prueba
        if (comentario != -1)// si hay un comentario, reduzco la linea hasta antes de el comentario
        {
            linea = linea.substr(0, comentario);
        }
        string::iterator it;
        for (it = linea.begin(); it != linea.end(); ++it) //recorro la linea sumando sus caracteres, para saber si solo tiene espacios en blanco
        {
            suma_chars += (*it);
            if(*it == ' ')
                cant_espacios ++;
            if (*it == 9)
                cant_tabs++;
        }
        cout << "SUMACHARS: " << suma_chars<<endl;
        // newline
        if (cant_espacios == linea.size()) // si es igual hay puro espacio y se reduce la linea
        {
            linea = "";
        }
        if (cant_tabs == linea.size()) // si es igual hay puro espacio y se reduce la linea
        {
            linea = "";
        }
        //cout << cant_espacios<<endl;
        lineas.push_back(linea);
    }

    archivo.close();
}

void Scanner::printLinea()
{
    for (int i = 0; i < lineas.size(); i++)
    {
        cout << "[";
        for (int j = 0; j < lineas[i].size(); j++)
        {
            //if (lineas[i][j] == '\n') cout << "ENDL";
            cout << lineas[i][j];
            
        }cout << "]" << lineas[i].size() << endl;
    }
}

void Scanner::imprimirTokens()
{
    vector<Token>::iterator it;
    for (it = tokens.begin(); it < tokens.end(); it++)
    {
        cout << "DEBUG SCAN - " << (*it).tipo << "  [" << (*it).valor << "] found at (" << (*it).fila << ":" << (*it).columna << ")" << endl;
    }
}

void Scanner::imprimirErrores()
{
    cout << "------------------------------------------" << endl;
    vector<Token>::iterator it;
    cout << errores.size();
    for (it = errores.begin(); it < errores.end(); it++)
    {
        cout << "INFO SCAN - " << (*it).tipo << "  [" << (*it).valor << "] found at (" << (*it).fila << ":" << (*it).columna << ") " << (*it).error << endl;
    }
}


int main()
{
    
    Scanner scan("archivo.txt");
    scan.leerArchivo();
    scan.printLinea();
    scan.escaner();
    scan.imprimirTokens();
    scan.imprimirErrores();

    return 0;
}
