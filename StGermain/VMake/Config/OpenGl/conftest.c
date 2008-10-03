#include <osmesa.h>
int main() {
	void* osMesaContext = OSMesaCreateContext( OSMESA_RGBA, 0 );
	OSMesaMakeCurrent( osMesaContext, 0, GL_UNSIGNED_BYTE, 100, 100 );
	OSMesaDestroyContext( osMesaContext );
	return 0;
}
