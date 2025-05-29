Los servers se crean en esta estructura:

struct ServersManager
    map<HostPort, vector<Servers> >

Contiene un:

struct HostPort
    string host
    int port
    operator==
    
O sea que ServersManager mapea "host:port":"vector<servidores>"

Cada request deberá obtener el value de su key "host:port".
Si hay más de uno entonces deberá mirar el servername.
Si no hay match, deberá usar el primer servidor del vector.
Hay una función miembro que recibe la request y devuelve el servidor adecuado.