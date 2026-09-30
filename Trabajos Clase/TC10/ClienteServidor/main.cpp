#include <cstdio>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include "Cliente.hpp"
#include "Producto.hpp"
#include "Carrito.hpp"

static std::vector<std::string> parsearCategorias( const std::string & body ) {
   std::vector<std::string> categorias;
   size_t pos = 0;
   while ( (pos = body.find("<A href=", pos)) != std::string::npos ) {
      size_t inicioTexto = body.find('>', pos);
      if ( inicioTexto == std::string::npos ) break;
      size_t finTexto = body.find("</A>", inicioTexto);
      if ( finTexto == std::string::npos ) break;
      categorias.push_back( body.substr(inicioTexto + 1, finTexto - inicioTexto - 1) );
      pos = finTexto + 4;
   }
   return categorias;
}

// Arma el body application/x-www-form-urlencoded que espera
// POST /TicAmazon/proforma.php: item0_categoria=...&item0_producto=...&item0_cantidad...
static std::string ArmarCuerpoProforma( const std::vector<ItemCarrito> & items ) {
   std::ostringstream cuerpo;
   for ( size_t i = 0; i < items.size(); ++i ) {
      if ( i > 0 ) cuerpo << "&";
      cuerpo << "item" << i << "_categoria=" << UrlEncode(items[i].producto.categoria)
             << "&item" << i << "_producto=" << UrlEncode(items[i].producto.descripcion)
             << "&item" << i << "_cantidad=" << items[i].cantidadElegida;
   }
   return cuerpo.str();
}

int main( int argc, char * argv[] ) {
   const char * host = (argc > 1) ? argv[1] : "os.ecci.ucr.ac.cr";
   const char * servicio = (argc > 2) ? argv[2] : "http";

   Cliente cliente;
   Carrito carrito;
   bool seguirComprando = true;

   while ( seguirComprando ) {
      
      std::vector<std::string> categorias;
      try {
         std::string bodyCategorias = cliente.Get(host, servicio, "/TicAmazon/list.php");
         categorias = parsearCategorias(bodyCategorias);
      } catch ( const std::runtime_error & e ) {
         std::cout << "Error al consultar categorias: " << e.what() << "\n";
         break;
      }

      if ( categorias.empty() ) {
         std::cout << "El servidor no reporta categorias disponibles.\n";
         break;
      }

      std::cout << "\nCategorias disponibles:\n";
      for ( size_t i = 0; i < categorias.size(); i++ ) {
         std::cout << "  [" << i << "] " << categorias[i] << "\n";
      }
      std::cout << "Elija el indice de la categoria: ";
      int idxCategoria;
      std::cin >> idxCategoria;
      std::cin.ignore();

      if ( idxCategoria < 0 || idxCategoria >= (int)categorias.size() ) {
         std::cout << "Indice invalido.\n";
      } else {
         std::string categoria = categorias[idxCategoria];
         std::string path = "/TicAmazon/list.php?category=" + UrlEncode(categoria);

         try {
            std::string body = cliente.Get(host, servicio, path);
            std::vector<Producto> productos = parsearProductos(body);

            if ( productos.empty() ) {
               std::cout << "No se encontraron productos en esa categoria.\n";
            } else {
               for ( size_t i = 0; i < productos.size(); i++ ) {
                  printf("[%zu] %s - stock: %d - precio: %.2f (bodega: %s)\n",
                         i, productos[i].descripcion.c_str(),productos[i].cantidad,
                         productos[i].precio, productos[i].bodega.c_str());
               }

               int indice, cantidad;
               std::cout << "Elija un indice de producto (-1 para ninguno): ";
               std::cin >> indice;

               if ( indice >= 0 && indice < (int)productos.size() ) {
                  std::cout << "Cantidad deseada: ";
                  std::cin >> cantidad;

                  if ( cantidad > productos[indice].cantidad ) {
                     std::cout << "No hay suficientes productos. Solo hay: "
                               << productos[indice].cantidad << "\n";
                  } else {
                     carrito.Agregar(productos[indice], cantidad);
                     std::cout << "Producto agregado al carrito.\n";
                  }
               }
               std::cin.ignore();
            }

         } catch ( const std::runtime_error & e ) {
            std::cout << "Error al consultar el servidor: " << e.what() << "\n";
            std::cout << "Puede intentar con otra categoria.\n";
         }
      }

      std::cout << "Quiere buscar otra categoria? (s/n): ";
      char resp;
      std::cin >> resp;
      std::cin.ignore();
      seguirComprando = (resp == 's' || resp == 'S');
   }

   carrito.MostrarFactura();  

   // Cierra el ciclo cliente-servidor manda el carrito real al servidor
   if ( !carrito.Items().empty() ) {
      std::string cuerpo = ArmarCuerpoProforma(carrito.Items());
      try {
         std::string respuesta = cliente.Post(host, servicio, "/TicAmazon/proforma.php", cuerpo);
         std::cout << "\n=== Proforma confirmada por el servidor ===\n" << respuesta << "\n";
      } catch ( const std::runtime_error & e ) {
         std::cout << "No se pudo confirmar la proforma con el servidor: " << e.what() << "\n";
      }
   }
   return 0;
}