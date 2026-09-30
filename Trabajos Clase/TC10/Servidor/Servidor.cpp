#include "Servidor.hpp"
#include "Socket.hpp"     

std::vector<Item> inventario = {
   { "Bodega-1", "Alimentos y bebidas", "banano",   100, 10.00  },
   { "Bodega-1", "Alimentos y bebidas", "manzana",   10, 300.00 },
   { "Bodega-1", "Salud y belleza",     "perfume",   33, 44.00  },
   { "Bodega-2", "Salud y belleza",     "shampoo",   22, 11.00  },
   { "Bodega-2", "Alimentos y bebidas", "arroz",     50, 15.00  },
   { "Bodega-2", "Electronica",         "pantalla",   5, 150.00 },
};

std::string Lower( std::string s ) {
   std::transform( s.begin(), s.end(), s.begin(), []( unsigned char c ){ return std::tolower( c ); } );
   return s;
}

std::string DecodificarUrl( const std::string & t ) {
   std::string r;
   for ( size_t i = 0; i < t.size(); ++i ) {
      if ( t[i] == '+' ) r += ' ';
      else if ( t[i] == '%' && i + 2 < t.size() + 0 && isxdigit( (unsigned char)t[i+1] ) && isxdigit( (unsigned char)t[i+2] ) ) {
         r += (char)strtol( t.substr( i + 1, 2 ).c_str(), nullptr, 16 );
         i += 2;
      } else r += t[i];
   }
   return r;
}

 
std::string CodificarUrl( const std::string & t ) {
   std::ostringstream o;
   for ( unsigned char c : t ) {
      if ( isalnum( c ) || c == '-' || c == '_' || c == '.' || c == '~' ) o << c;
      else o << '%' << std::uppercase << std::hex << std::setw( 2 ) << std::setfill( '0' ) << (int)c;
   }
   return o.str();
}

std::map<std::string, std::string> ParsearForm( const std::string & texto ) {
   std::map<std::string, std::string> m;
   std::stringstream ss( texto );
   std::string par;
   while ( std::getline( ss, par, '&' ) ) {
      if ( par.empty() ) continue;
      size_t eq = par.find( '=' );
      std::string k = ( eq == std::string::npos ) ? par : par.substr( 0, eq );
      std::string v = ( eq == std::string::npos ) ? "" : par.substr( eq + 1 );
      m[ DecodificarUrl( k ) ] = DecodificarUrl( v );
   }
   return m;
}

std::string Decimal( double v ) {
   char b[64];
   snprintf( b, sizeof( b ), "%.2f", v );
   return b;
}

// HTML donde el mismo formato que espera el cliente
std::string Inicio( const std::string & t ) {
   return "<!DOCTYPE html>\n<HTML>\n<HEAD><meta charset=\"utf-8\"><TITLE>" + t + "</TITLE></HEAD>\n<BODY>\n<H1>" + t + "</H1>\n";
}
static const std::string FIN = "</BODY>\n</HTML>\n";

std::string PaginaCategorias() {
   std::vector<std::string> cats;
   for ( const Item & i : inventario )
      if ( std::find( cats.begin(), cats.end(), i.categoria ) == cats.end() ) cats.push_back( i.categoria );

   std::string h = Inicio( "TicAmazon - Categorias" ) + "<UL>\n";
   for ( const std::string & c : cats )
      h += "<LI><A href=\"/TicAmazon/list.php?category=" + CodificarUrl( c ) + "\">" + c + "</A></LI>\n";
   return h + "</UL>\n" + FIN;
}

std::string PaginaProductos( const std::string & cat ) {
   std::string h = Inicio( "TicAmazon - " + cat ) + "<TABLE border=\"1\">\n";
   h += "<TR><TH>Intermediario</TH><TH>Bodega</TH><TH>Categoria</TH><TH>Descripcion</TH><TH>Cantidad</TH><TH>Precio</TH></TR>\n";
   for ( const Item & i : inventario ) {
      if ( Lower( i.categoria ) != Lower( cat ) ) continue;
      h += "<TR><TD></TD><TD>" + i.bodega + "</TD><TD>" + i.categoria + "</TD><TD>" + i.descripcion +
           "</TD><TD>" + std::to_string( i.cantidad ) + "</TD><TD>" + Decimal( i.precio ) + "</TD></TR>\n";
   }
   return h + "</TABLE>\n" + FIN;
}

// Devuelve el codigo HTTP
int PaginaProforma( const std::string & cuerpo, std::string & html ) {
   auto campos = ParsearForm( cuerpo );
   std::string filas, errores;
   double total = 0;

   for ( int n = 0; campos.count( "item" + std::to_string( n ) + "_categoria" ); ++n ) {
      std::string p = "item" + std::to_string( n ) + "_";
      std::string cat = campos[ p + "categoria" ], desc = campos[ p + "producto" ];
      int cant = 0;
      try { cant = std::stoi( campos[ p + "cantidad" ] ); } catch ( ... ) { cant = -1; }

      const Item * enc = nullptr;
      for ( const Item & i : inventario )
         if ( Lower( i.categoria ) == Lower( cat ) && Lower( i.descripcion ) == Lower( desc ) ) { enc = &i; break; }

      if ( cant <= 0 )                 errores += "<LI>Cantidad invalida para '" + desc + "'</LI>\n";
      else if ( !enc )                 errores += "<LI>El producto '" + desc + "' no existe en '" + cat + "'</LI>\n";
      else if ( cant > enc->cantidad ) errores += "<LI>Stock insuficiente para '" + desc + "'</LI>\n";
      else {
         double sub = cant * enc->precio;
         total += sub;
         filas += "<TR><TD>" + enc->bodega + "</TD><TD>" + enc->categoria + "</TD><TD>" + enc->descripcion +
                  "</TD><TD>" + std::to_string( cant ) + "</TD><TD>" + Decimal( enc->precio ) +
                  "</TD><TD>" + Decimal( sub ) + "</TD></TR>\n";
      }
   }

   html = Inicio( "TicAmazon - Factura proforma" );
   if ( !errores.empty() || filas.empty() ) {
      html += "<P>No se pudo generar la proforma:</P>\n<UL>\n" + ( errores.empty() ? "<LI>Sin productos</LI>\n" : errores ) + "</UL>\n" + FIN;
      return 422;
   }
   html += "<TABLE border=\"1\">\n<TR><TH>Bodega</TH><TH>Categoria</TH><TH>Descripcion</TH><TH>Cantidad</TH><TH>Precio</TH><TH>Subtotal</TH></TR>\n";
   html += filas + "</TABLE>\n<P><B>TOTAL: " + Decimal( total ) + "</B></P>\n" + FIN;
   return 200;
}

// Reconocer la respuesta
std::string Respuesta( int codigo, const std::string & cuerpo ) {
   std::string frase = codigo == 200 ? "OK" : codigo == 404 ? "Not Found" : codigo == 405 ? "Method Not Allowed"
                     : codigo == 422 ? "Unprocessable Entity" : "Bad Request";
   return "HTTP/1.1 " + std::to_string( codigo ) + " " + frase + "\r\n"
          "Content-Type: text/html; charset=utf-8\r\n"
          "Content-Length: " + std::to_string( cuerpo.size() ) + "\r\n"
          "Connection: close\r\n\r\n" + cuerpo;
}

//Leer solicitud
std::string LeerRequest( Socket & s ) {
   std::string acc;
   char buf[1024];
   size_t fin = std::string::npos;
   while ( ( fin = acc.find( "\r\n\r\n" ) ) == std::string::npos ) {
      size_t n = s.Read( buf, sizeof( buf ) );
      if ( n == 0 ) return acc;
      acc.append( buf, n );
   }
   // Si hay Content-Length, leer el resto del body
   std::string h = Lower( acc.substr( 0, fin ) );
   size_t cl = h.find( "content-length:" );
   if ( cl != std::string::npos ) {
      size_t largo = (size_t)std::atoi( h.c_str() + cl + 15 );
      while ( acc.size() < fin + 4 + largo ) {
         size_t n = s.Read( buf, sizeof( buf ) );
         if ( n == 0 ) break;
         acc.append( buf, n );
      }
   }
   return acc;
}

void Atender( Socket & s ) {
   std::string req = LeerRequest( s );
   if ( req.empty() ) return;

   std::istringstream linea( req.substr( 0, req.find( "\r\n" ) ) );
   std::string metodo, url, version;
   linea >> metodo >> url >> version;

   size_t q = url.find( '?' );
   std::string ruta = url.substr( 0, q );
   auto query = ParsearForm( q == std::string::npos ? "" : url.substr( q + 1 ) );
   size_t fh = req.find( "\r\n\r\n" );
   std::string cuerpo = ( fh == std::string::npos ) ? "" : req.substr( fh + 4 );

   std::string resp, html;
   if ( metodo == "GET" && ruta == "/TicAmazon/list.php" ) {
      auto it = query.find( "category" );
      resp = Respuesta( 200, it == query.end() ? PaginaCategorias() : PaginaProductos( it->second ) );
   } else if ( metodo == "POST" && ruta == "/TicAmazon/proforma.php" ) {
      int codigo = PaginaProforma( cuerpo, html );
      resp = Respuesta( codigo, html );
   } else if ( ruta == "/TicAmazon/list.php" || ruta == "/TicAmazon/proforma.php" ) {
      resp = Respuesta( 405, Inicio( "Error 405" ) + "<P>Metodo no soportado</P>\n" + FIN );
   } else {
      resp = Respuesta( 404, Inicio( "Error 404" ) + "<P>Ruta no encontrada</P>\n" + FIN );
   }

   size_t enviados = 0;
   while ( enviados < resp.size() ) {
      size_t n = s.Write( (const void *)( resp.data() + enviados ), resp.size() - enviados );
      if ( n == 0 ) break;
      enviados += n;
   }
   std::cout << metodo << " " << url << "\n";
}

int main( int argc, char * argv[] ) {
   int puerto = ( argc > 1 ) ? std::atoi( argv[1] ) : 8080;
   signal( SIGPIPE, SIG_IGN );   // evita que el proceso muera si el cliente cierra antes

   try {
      Socket escucha( 's', false );
      escucha.Bind( puerto );
      escucha.Listen( 8 );
      std::cout << "Servidor simple escuchando en el puerto " << puerto << "\n";

      while ( true ) {
         try {
            Socket cliente = escucha.Accept();
            Atender( cliente );      // al salir del scope, el destructor cierra la conexion
         } catch ( const std::exception & e ) {
            std::cerr << "Error atendiendo conexion: " << e.what() << "\n";
         }
      }
   } catch ( const std::exception & e ) {
      std::cerr << "No se pudo iniciar: " << e.what() << "\n";
      return 1;
   }
}