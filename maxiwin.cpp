#if defined(_WIN32)
#define _MAXIWIN_H_TO_CPP_

//////////////////////////////// Librerias y Funciones metodo //////////////////////////////

#include <windows.h>
#include <gdiplus.h>

#ifndef _MAXIWIN_H_
#include "maxiwin.h"
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_PI2
#define M_PI2 6.28318530717958647692
#endif
#ifndef M_ONEQTR_PI
#define M_ONEQTR_PI 0.785398163397448309615
#endif
#ifndef M_THRQTR_PI
#define M_THRQTR_PI 2.356194490192344928845
#endif
#ifndef M_DEG2RAD
#define M_DEG2RAD(x) (x*0.01745329251994329576) // x * (π / 180)
#endif
#ifndef M_RAD2DEG
#define M_RAD2DEG(x) (x*57.2957795130823208767) // x * (180 / π)
#endif

#ifndef __clamp
template<typename T>inline T __clamp(T val, T min_val, T max_val){return(val<min_val)?min_val:(val>max_val)?max_val:val;}
#endif

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "gdiplus.lib")

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
namespace maxiwin{
	struct fnt{
		DynArray<char> nombre;
		int dimension, angulo;
		bool italica, subrayada, negrita, monoespacio;
		fnt();
		fnt(decltype(nullptr)) : fnt() {}
		fnt(const DynArray<char>& _name);
		fnt(const DynArray<wchar_t>& _wname) : fnt(DynArray<char>(_wname.begin(), _wname.end())) {}
	};
	struct pnt{
		float x, y, z;
	    pnt(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}
		pnt(decltype(nullptr)) : pnt() {}
	    pnt operator-() const{ return {-x, -y, -z}; }
	    pnt operator+(const pnt& o) const{ return {x + o.x, y + o.y, z + o.z}; }
	    pnt operator-(const pnt& o) const{ return {x - o.x, y - o.y, z - o.z}; }
	    pnt& operator+=(const pnt& o){ x += o.x; y += o.y; z += o.z; return *this; }
	    pnt& operator-=(const pnt& o){ x -= o.x; y -= o.y; z -= o.z; return *this; }
	    // Escalares
	    pnt operator*(float f) const{ return {x * f, y * f, z * f}; } pnt& operator*=(float f){ return ((*this) = (*this) * f); }
	    friend pnt operator*(float f, const pnt& i){ return i * f; } friend pnt& operator*=(float f, pnt& i){ return i *= f; }
	    pnt operator/(float f) const{ return {x / f, y / f, z / f}; } pnt& operator/=(float f){ return ((*this) = (*this) / f); }
	    float operator*(const pnt& o) const{ return x * o.x + y * o.y + z * o.z; } pnt& operator*=(const pnt& o){ return ((*this) = (*this) * o); }
	    // Vectoriales
	    static float dot(const pnt& a, const pnt& b){ return a.x * b.x + a.y * b.y + a.z * b.z; }
	    float dot(const pnt& o) const{ return o.x * x + o.y * y + o.z * z; }
		static pnt cruz(const pnt& a, const pnt& b){ return {
		    a.y * b.z - a.z * b.y,
		    a.z * b.x - a.x * b.z,
		    a.x * b.y - a.y * b.x
		}; }
		pnt cruz(const pnt& o) const{ return {
		    y * o.z - z * o.y,
		    z * o.x - x * o.z,
		    x * o.y - y * o.x
		}; }
		float distancia(const pnt& o) const;
		float longitud() const{ return distancia({0, 0, 0}); }
		pnt normalizar() const{
		    float len = longitud();
		    return (len == 0) ? pnt{} : ((*this) / len);
		}
		// Comparadores
		bool operator==(const pnt& o) const{ return (x == o.x && y == o.y && z == o.z); }
		bool operator!=(const pnt& o) const{ return !((*this) == o); }
	};
	struct cam{
		pnt posicion = {0, 0, 0},
			objetivo = {0, 0, 1}, // distancia focal de 128
			up       = {0, 1, 0};
		float fov = 90.0f,
			  z_near = 0.0f,    // Plano cercano
	    	  z_far  = 1000.0f; // Plano lejano
	    cam() = default;
		cam(decltype(nullptr)) : cam() {}
	    cam(pnt pos, pnt obj = {0, 0, 1}, float _fov = 90.0f) : posicion(pos), objetivo(obj), fov(_fov) {}
	    // Utilidades futuras posibles
	    float focal() const{
			static pnt post_point = objetivo;
			static float post_result = 128.0f;
			if(objetivo == post_point) return post_result;
			post_point = objetivo;
			post_result = 128.0f * objetivo.longitud();
			return post_result;
		}
	};
	struct ParamStack{
		const char* szClassName;
		HWND* hWnd;
		int* iWidth;
		int* iHeight;
		int* all_ready;
		void (*frame_real)(int,int,int&,int&);
		LRESULT CALLBACK (*WndProc)(HWND,UINT,WPARAM,LPARAM);
	};
	DWORD WINAPI PThreadPermanent(LPVOID lpParam){
		ParamStack* GlobalData = (ParamStack*)lpParam;
		HINSTANCE hInstance = GetModuleHandle(NULL);
		WNDCLASSEX wc = {};
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.lpfnWndProc = GlobalData->WndProc;
		wc.hInstance = hInstance;
		wc.lpszClassName = GlobalData->szClassName;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = CreateSolidBrush(RGB(0,0,0));
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
		if(!RegisterClassEx(&wc)){
		    MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		    return 0;
		}
		int w, h; GlobalData->frame_real(*(GlobalData->iWidth), *(GlobalData->iHeight), w, h);
		*(GlobalData->hWnd) = CreateWindowEx(
			0,
			GlobalData->szClassName,
			GlobalData->szClassName,
		    WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, w, h,
		    HWND_DESKTOP, NULL, hInstance, NULL
		);
		if(!*(GlobalData->hWnd)){
		    MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		    return 0;
		}
		ShowWindow(*(GlobalData->hWnd), SW_SHOW);
		UpdateWindow(*(GlobalData->hWnd));
	    *(GlobalData->all_ready) = 1;
	    MSG g_msg; while(GetMessage(&g_msg, NULL, 0, 0) && *(GlobalData->all_ready) == 1){
	        TranslateMessage(&g_msg);
	        DispatchMessage(&g_msg);
	    }
	    return 0;
	}
	void frame_real(int w, int h, int& rw, int &rh){
		RECT frame = { 0, 0, w, h };
		AdjustWindowRect(&frame, WS_OVERLAPPEDWINDOW, FALSE);
		rw = frame.right - frame.left;
		rh = frame.bottom - frame.top;
	}
	struct Initializer{ // Estructura que maneja la creación de la ventana y la ejecución en paralelo
		//////////////////////////////////// Variables globales ////////////////////////////////////
		ULONG_PTR gdiplusToken;
		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		void iniciarGDIPlus(){ Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr); }
		void cerrarGDIPlus() { Gdiplus::GdiplusShutdown(gdiplusToken); }
		const char* szClassName = "MaxiWin";
		// Valores de lectura
		int	iXpos = 100, iYpos = 50,               // Posicion Horizontal y Vertical de la ventana
			iWidth = 400, iHeight = 300;           // Ancho y Alto de la ventana
		int _xraton = 0, _yraton = 0;              // Posicion del raton
		bool _raton_dentro = false,                // "¿El raton está dentro del 'client area'?"
			 _bot_izq = false, _bot_der = false;   // Botones izquierdo y derecho
		bool* _teclas_osadas;                      // Teclas de escritura
		bool* _teclas_precionadas;                 // Teclas de continua accion
		// Soporte
		int all_ready = 0;       // Controlador de la ventana en 'Initializer'
		HWND hWnd;                    // Ventana principal
		Callback<> quake_exit;        // Funcion solicita por el programador, cuando se intenta cerrar la ventana
		bool cancel_clossing = false; // "¿Se solicito ejecutar algo, cuando la ventana intenta ser cerrada?"
		DWORD GetThreadID; HANDLE hThread;
		// Administradores
		Gdiplus::Bitmap* backBuffer = nullptr;      // Buffer
		Gdiplus::Graphics* grafics = nullptr;       // Dibujante
		Gdiplus::Color _color = Gdiplus::Color(255, 255, 255, 255);  // Color
		Gdiplus::SolidBrush* brush = nullptr;       // Area de relleno
		Gdiplus::Pen* pen = nullptr;                // Lapiz
		Gdiplus::PrivateFontCollection g_fuentes;   // Fuentes globales
		DynArray<DynArray<wchar_t>> g_nombres_familia;
		size_t g_fuentes_defecto = 0;
		DynArray<float> zbuffer;                    // Z-Buffer para graficos 3D
		ParamStack* myStack;
		// Estructuras almacenadoras
		fnt g_fuente_actual;
		cam _g_cam;
		
		void newMemDC(int w, int h){
			Gdiplus::Bitmap* resized = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
			if(grafics) delete grafics; grafics = new Gdiplus::Graphics(resized);
			grafics->SetCompositingMode(Gdiplus::CompositingModeSourceOver);
			grafics->Clear(Gdiplus::Color(255, 0, 0, 0));
			if(backBuffer){
				grafics->DrawImage(backBuffer, 0, 0);
				delete backBuffer;
			}
			backBuffer = resized;
			zbuffer.clear();
			zbuffer.resize(w * h);
			zbuffer.set(255.0f);
		}
		int __getting_key(int key){
			// Instant system exceptions and translations
			if( (key >= '0' && key <= '9') ||
				(key >= 'A' && key <= 'Z') ||
				(key >= 'a' && key <= 'z') ||
				key == '?' || key == '!') return key;
			if(key >= VK_NUMPAD0 && key <= VK_NUMPAD9) return key - VK_NUMPAD0 + '0';
			#ifdef _MAXIWIN_H_
			if(key >= VK_F1 && key <= VK_F12) return key - VK_F1 + maxiwin::teclas::F1;
			#endif
			switch(key){
				#ifdef _MAXIWIN_H_
				case     VK_ESCAPE: return maxiwin::teclas::ESCAPE;
				case       VK_LEFT: return maxiwin::teclas::IZQUIERDA;
				case      VK_RIGHT: return maxiwin::teclas::DERECHA;
				case         VK_UP: return maxiwin::teclas::ARRIBA;
				case       VK_DOWN: return maxiwin::teclas::ABAJO;
				case      VK_SPACE: return maxiwin::teclas::ESPACIO;
				case     VK_RETURN: return maxiwin::teclas::RETURN;
				case       VK_BACK: return maxiwin::teclas::BACKSPACE;
				case      VK_SHIFT: return maxiwin::teclas::SHIFT;
				case    VK_CONTROL: return maxiwin::teclas::CTRL;
				case       VK_MENU: return maxiwin::teclas::ALT;
				case        VK_TAB: return maxiwin::teclas::TAB;
				case    VK_CAPITAL: return maxiwin::teclas::MAYUSCULAS;
				case     VK_DELETE: return maxiwin::teclas::SUPR;
				case       VK_LWIN: return maxiwin::teclas::LWIN;
				case       VK_RWIN: return maxiwin::teclas::RWIN;
				case       VK_APPS: return maxiwin::teclas::APP;
				#endif
				case      VK_OEM_1: return ';';
				case   VK_OEM_PLUS: return '+';
				case  VK_OEM_COMMA: return ',';
				case  VK_OEM_MINUS: return '-';
				case VK_OEM_PERIOD: return '.';
				case      VK_OEM_2: return '/';
				case      VK_OEM_3: return '`';
				case      VK_OEM_4: return '[';
				case      VK_OEM_5: return '\\';
				case      VK_OEM_6: return ']';
				case      VK_OEM_7: return '\'';
			}
			bool parrsing = false; // Evaluation variable
			// Preprossesed keys
			#ifdef _MAXIWIN_H_
			parrsing |= key == maxiwin::teclas::ESCAPE;
			parrsing |= key == maxiwin::teclas::IZQUIERDA;
			parrsing |= key == maxiwin::teclas::DERECHA;
			parrsing |= key == maxiwin::teclas::ARRIBA;
			parrsing |= key == maxiwin::teclas::ABAJO;
			parrsing |= key == maxiwin::teclas::ESPACIO;
			parrsing |= key == maxiwin::teclas::RETURN;
			parrsing |= key == maxiwin::teclas::BACKSPACE;
			parrsing |= key == maxiwin::teclas::SHIFT;
			parrsing |= key == maxiwin::teclas::CTRL;
			parrsing |= key == maxiwin::teclas::ALT;
			parrsing |= key == maxiwin::teclas::TAB;
			parrsing |= key == maxiwin::teclas::MAYUSCULAS;
			parrsing |= key == maxiwin::teclas::LWIN;
			parrsing |= key == maxiwin::teclas::RWIN;
			parrsing |= key == maxiwin::teclas::APP;
			#endif
			// Decorator keys
			const wchar_t decore_list[] = { // ñÑçÇœŒáéíóúýäëïöüÿâêîôûÁÉÍÓÚÝÄËÏÖÜÂÊÎÔÛ¿¡
				L'\u00F1', L'\u00D1', L'\u00E7', L'\u00C7', L'\u0153', L'\u0152', L'\u00E1', L'\u00E9',
				L'\u00ED', L'\u00F3', L'\u00FA', L'\u00FD', L'\u00E4', L'\u00EB', L'\u00EF', L'\u00F6',
			    L'\u00FC', L'\u00FF', L'\u00E2', L'\u00EA', L'\u00EE', L'\u00F4', L'\u00FB', L'\u00C1',
			    L'\u00C9', L'\u00CD', L'\u00D3', L'\u00DA', L'\u00DD', L'\u00C4', L'\u00CB', L'\u00CF',
			    L'\u00D6', L'\u00DC', L'\u00C2', L'\u00CA', L'\u00CE', L'\u00D4', L'\u00DB', L'\u00BF',
			    L'\u00A1'
			}; const size_t decore_list_size = sizeof(decore_list) / sizeof(decore_list[0]);
			for(size_t i = 0; i < decore_list_size; i++) parrsing |= key == decore_list[i];
			if(parrsing) return key;
			#ifdef _MAXIWIN_H_
			return maxiwin::teclas::NINGUNA;
			#else
			return 0;
			#endif
		}
		void __teclas_actualizar(int key, bool value){
			int subkey = __getting_key(key);
			if(subkey != 0){
				_teclas_osadas[subkey] = value; // Pulsaciones breves
				_teclas_precionadas[subkey] = value; // Pulsacion continua
				if(!value){
					_teclas_osadas[0] = false; _teclas_precionadas[0] = false;
					for(int i=1; i < 256; i++){ _teclas_osadas[0] |= _teclas_osadas[i]; _teclas_precionadas[0] |= _teclas_precionadas[i]; }
					_teclas_osadas[0] ^= true; _teclas_precionadas[0] ^= true; // inversion
				}
			}
		}
	    int power_on(){
			_teclas_osadas = new bool[256]{false};
	    	_teclas_precionadas = new bool[256]{false};
	    	all_ready = 0;
	    	iniciarGDIPlus();
	    	newMemDC(iWidth, iHeight);
		    brush = new Gdiplus::SolidBrush(_color);
		    pen = new Gdiplus::Pen(_color, 1.0f);
			myStack = new ParamStack;
			myStack->szClassName = szClassName;
			myStack->hWnd = &hWnd;
			myStack->iWidth = &iWidth;
			myStack->iHeight = &iHeight;
			myStack->all_ready = &all_ready;
			myStack->frame_real = frame_real;
			myStack->WndProc = WndProc;
		    hThread = CreateThread(
		    	NULL,
		    	0,
		    	PThreadPermanent,
		    	myStack,
		    	0,
		    	&GetThreadID
			);
			if(hThread == NULL){
		        fprintf(stderr, "Error al crear el hilo.\n");
		        delete myStack;
		        Sleep(5000);
		        abort();
			}
			do{ Sleep(100); }while(!IsWindow(hWnd) || !all_ready);
			maxiwin::borra(); // Limpia la ventana antes de empezar.
			// Cargar fuentes del sistema
			Gdiplus::InstalledFontCollection sistema;
		    g_fuentes_defecto = sistema.GetFamilyCount();
		    if(g_fuentes_defecto > 0){
			    Gdiplus::FontFamily* familias = new Gdiplus::FontFamily[g_fuentes_defecto];
			    int numEncontradas  = g_fuentes_defecto;
			    if(sistema.GetFamilies(numEncontradas, familias, &numEncontradas) == Gdiplus::Ok){
			        for(int i = 0; i < numEncontradas; i++){
			        	wchar_t nombre[LF_FACESIZE]{L'\0'};
			            if(familias[i].GetFamilyName(nombre) == Gdiplus::Ok){
						    size_t longitud = 0;
						    while(nombre[longitud] != L'\0') longitud++;
						    DynArray<wchar_t> copia;
						    copia.resize(longitud + 1);
						    for(size_t j = 0; j < longitud; j++) copia[j] = nombre[j];
						    copia[longitud] = L'\0';
						    g_nombres_familia.emplace_back(copia);
						}
					}
			    }
			    g_fuentes_defecto = numEncontradas;
			    delete[] familias;
			}
			g_fuentes_defecto = g_nombres_familia.size();
			if(!g_nombres_familia.empty()) g_fuente_actual = fnt(g_nombres_familia[0]);
			return 0;
		}
	}; Initializer* stared;
	struct Initializer_part_2{
		Initializer_part_2(){ stared = new Initializer(); stared->power_on(); }
		~Initializer_part_2(){
			do{
			    Sleep(100);
			    MSG msg; while(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)){
			        TranslateMessage(&msg);
			        DispatchMessage(&msg);
			    }
			}while(IsWindow(stared->hWnd) && stared->all_ready);
		    CloseHandle(stared->hThread);
			stared->cerrarGDIPlus();
			delete stared;
		}
	} post_stared;
	fnt::fnt() : nombre((stared)?((stared->g_nombres_familia.empty())?nullptr:stared->g_nombres_familia[0].data()):nullptr), angulo(0), dimension(12),
			italica(false), subrayada(false), negrita(false), monoespacio(true) {}
	fnt::fnt(const DynArray<char>& _name) : nombre((stared)?((stared->g_nombres_familia.empty())?nullptr:stared->g_nombres_familia[0].data()):nullptr) {
		angulo = 0; dimension = 12;
		bool founded = false;
		for(size_t i=0; i < stared->g_nombres_familia.size(); i++){
			DynArray<char> wname(stared->g_nombres_familia[i].begin(), stared->g_nombres_familia[i].end());
			if(wname == _name){
				nombre = wname;
				founded = true;
				break;
			}
		}
		italica = subrayada = negrita = monoespacio = false;
	}
	
	inline int round_x(const float& x){ return (x == (float)(int)x) ? x : x + 0.5f; }
	inline int floor_x(const float& x){ return (x == (float)(int)x) ? x : ((x > 0.0f) ? (int)x : (int)x + 1.0f); }
	inline int ceil__x(const float& x){ return (x == (float)(int)x) ? x : ((x > 0.0f) ? (int)x + 1.0f : (int)x); }
	inline float fmod__x(float x, const float& y){
		while(x <  0) x += y;
		while(x >= y) x -= y;
		return x;
	}
	template<class InputIterator, class T> inline InputIterator __find(InputIterator first, InputIterator last, const T& val){
		while(first != last){
	    	if(*first==val) return first;
			++first;
		}
		return last;
	}
	template<class InputIterator, class T> inline size_t __count(InputIterator first, InputIterator last, const T& val){
		size_t ret = 0;
		while(first != last){
			if(*first == val) ++ret;
			++first;
		}
		return ret;
	}
	template<class ForwardIterator, class T> inline void __replace(ForwardIterator first, ForwardIterator last, const T& old_value, const T& new_value){
		while(first != last){
			if(*first == old_value) *first = new_value;
			++first;
		}
	}
	template<class InputIterator, class OutputIterator, class UnaryOperator> inline OutputIterator __transform(InputIterator first1, InputIterator last1,
	  OutputIterator result, UnaryOperator op){
		while(first1 != last1){
			*result = op(*first1);
			++result; ++first1;
		}
		return result;
	}
	inline float square_root(float number){
	    union{
	        float f;
	        uint32_t i;
	    } conv = { number };
	    conv.i = 0x5f3759df - (conv.i >> 1); // Numero magico de Quake III
	    float y = conv.f;
	    y = y * (1.5f - 0.5f * number * y * y); // Tres doritos despues...
	    y = y * (1.5f - 0.5f * number * y * y);
	    y = y * (1.5f - 0.5f * number * y * y);
	    return number * y;
	}
	float pnt::distancia(const pnt& o) const{
		return square_root(
			(o.x - x) * (o.x - x) +
			(o.y - y) * (o.y - y) +
			(o.z - z) * (o.z - z)
		);
	}
	inline float fast_atan2(float y, float x){
		float r, angle;
		float abs_y = ((y < 0.0f) ? -y : y) + 1e-10f; // evita división por cero
		if(x < 0.0f){
			r = (x + abs_y) / (abs_y - x);
			angle = M_THRQTR_PI;
		}else{
			r = (x - abs_y) / (x + abs_y);
			angle = M_ONEQTR_PI;
		}
		angle += (0.1963f * r * r - 0.9817f) * r;
		if(y < 0.0f) return -angle;
		return angle;
	}
	template<class ForwardIterator> inline void fast_sort(ForwardIterator first, ForwardIterator last){
	    if(first == last) return;
	    auto distance = last - first;
	    if(distance <= 1) return;
	    using T = typename mini_traits::remove_cv_ref<decltype(*first)>::type;
	    T pivot = *(first + distance / 2);
	    ForwardIterator i = first;
	    ForwardIterator j = last - 1;
	    while(i <= j){
	        while (*i < pivot) ++i;
	        while (*j > pivot) --j;
	        if(i <= j){
	            T tmp = *i;
	            *i = *j;
	            *j = tmp;
	            ++i;
	            if (j > first) --j;
	        }
	    }
	    if(first < j) __sort(first, j + 1);
	    if(i < last)  __sort(i, last);
	}
	template<class ForwardIterator> inline void fast_sort(
		ForwardIterator first,
		ForwardIterator last,
		bool (*cmp)(
			const typename mini_traits::iterator_traits<ForwardIterator>::value_type&,
			const typename mini_traits::iterator_traits<ForwardIterator>::value_type&)
		){
	    if(first == last) return;
	    auto distance = last - first;
	    if(distance <= 1) return;
	    using T = typename mini_traits::iterator_traits<ForwardIterator>::value_type;
	    T pivot = *(first + (distance >> 1));
	    ForwardIterator i = first, j = last - 1;
	    while(i <= j){
	        while(cmp(*i, pivot)) ++i;
	        while(cmp(pivot, *j)) --j;
	        if(i <= j){
	            T tmp = *i;
	            *i = *j;
	            *j = tmp;
	            ++i;
	            if (j > first) --j;
	        }
	    }
	    if(first < j) fast_sort(first, j + 1, cmp);
	    if(i < last)  fast_sort(i, last, cmp);
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	//                                                                                        //
	//                                  Funciones del MOTOR                                   //
	//                                                                                        //
	////////////////////////////////////////////////////////////////////////////////////////////
	class img{
		bool im_a_copy = false;
		public:
			unsigned int ancho, alto;
			uint32_t* contenido;
			img() : ancho(0), alto(0), contenido(nullptr), im_a_copy(false) {}
			img(decltype(nullptr)) : img() {}
			img(unsigned int w, unsigned int h, uint32_t* c) : ancho(w), alto(h), contenido(c), im_a_copy(true) {}
			img(unsigned int w, unsigned int h) : ancho(w), alto(h), contenido(new uint32_t[w * h]), im_a_copy(false)
				{ memset(contenido, 0, sizeof(uint32_t) * ancho * alto); }
			img(const img& other) : ancho(other.ancho), alto(other.alto), contenido(new uint32_t[other.ancho * other.alto]) {
				if(contenido && other.contenido) for(size_t i=0,j=ancho * alto;i<j;i++)contenido[i]=other.contenido[i]; }
		    img(img&& other) noexcept : ancho(other.ancho), alto(other.alto), contenido(other.contenido) {
			    other.ancho = 0;
			    other.alto = 0;
			    other.contenido = nullptr;
			}
		    ~img(){ if(!im_a_copy) delete[] contenido; }
			img& operator=(const img& other){
			    if(this == &other) return *this;
			    delete[] contenido;
			    ancho = other.ancho;
			    alto = other.alto;
			    contenido = new uint32_t[ancho * alto];
			    for(size_t i=0,j=ancho * alto;i<j;i++)contenido[i]=other.contenido[i];
			    return *this;
			}
		    img& operator=(img&& other) noexcept{
			    if(this == &other) return *this;
			    delete[] contenido;
			    ancho = other.ancho;
			    alto = other.alto;
			    contenido = other.contenido;
			    other.ancho = 0;
			    other.alto = 0;
			    other.contenido = nullptr;
			    return *this;
			}
			bool operator==(const img& other) const{
			    if(ancho != other.ancho || alto != other.alto) return false;
			    for(size_t i=0,j=ancho * alto;i<j;i++)if(contenido[i]!=other.contenido[i])return false;
			    return true;
			}
			bool operator!=(const img& other) const{ return !((*this) == other); }
			inline bool valido()const{return contenido&&ancho!=0&&alto!=0;}
			bool contiene(const uint32_t& _c)const{return valido()?(__find(contenido,contenido+ancho*alto,_c)!=contenido+ancho*alto):false;}
			bool contiene_unicamente(const uint32_t& _c) const{return valido()?(__count(contenido,contenido+ancho*alto,_c)==ancho*alto):false;}
			uint32_t& at(int x, int y){return contenido[y * ancho + x];}
		    const uint32_t& at(int x, int y) const{return contenido[y * ancho + x];}
		    img& remplazar(const uint32_t& _dst, const uint32_t& _src){
				for(size_t i=0,maxItr=ancho*alto;valido()&&_src!=_dst&&i<maxItr;i++)contenido[i]=(contenido[i]==_dst?contenido[i]:_src);
			    return *this;
			}
		    img& espejo(bool horz, bool vert){
		    	if(!valido()||(!horz&&!vert)) return *this;
		    	uint32_t* cache=new uint32_t[ancho*alto];
				for(size_t y=0;y<alto;y++) for(size_t x=0;x<ancho;x++){
					size_t nx=(horz?(ancho-1-x):x), ny=(vert?(alto-1-y):y);
					cache[y*ancho+x]=contenido[ny*ancho+nx];
				}
				delete[] contenido; contenido = cache;
				return *this;
			}
		    img recorte(int x, int y, int w, int h) const{
			    if(!contenido || ancho <= 0 || alto <= 0) return {};
			    if(x == 0 && y == 0 && w == ancho && h == alto) return *this;
			    if(x < 0){ w += x; x = 0; }
			    if(y < 0){ h += y; y = 0; }
			    if(x + w > ancho) w = ancho - x;
			    if(y + h > alto)  h = alto - y;
			    if(w <= 0 || h <= 0) return *this;
			    img result(w, h); for(size_t i=0;i<h;i++) for(size_t j=0;j<w;j++) result.at(j,i)=at(x+j,y+i);
			    return result;
			}
		    img& estirar(int w, int h){
				if(w == 0 || h == 0 || !contenido) return *reinterpret_cast<img*>(0);
			    if(w == ancho && h == alto) return *this;
			    const unsigned int newW = (w < 0) ? -w : w;
				const unsigned int newH = (h < 0) ? -h : h;
			    Gdiplus::Bitmap bmpOrigen(ancho, alto, ancho * 4, PixelFormat32bppARGB, (BYTE*)contenido);
			    if(bmpOrigen.GetLastStatus() != Gdiplus::Ok) return *this;
				uint32_t* new_content = new uint32_t[newW * newH];
				Gdiplus::Bitmap bmpDestino(newW, newH, newW * 4, PixelFormat32bppARGB, (BYTE*)new_content);
				if(bmpDestino.GetLastStatus() != Gdiplus::Ok){
					delete[] new_content;
					return *this;
				}
				{
					Gdiplus::Graphics graphics(&bmpDestino);
					graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
					graphics.SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);
					graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
					if(w < 0 || h < 0){ // Preparar transformación si hay volteo
						Gdiplus::Matrix transform;
						transform.Translate(w < 0 ? float(newW) : 0, h < 0 ? float(newH) : 0);
						transform.Scale(w < 0 ? -1.0f : 1.0f, h < 0 ? -1.0f : 1.0f);
						graphics.SetTransform(&transform);
					}
					graphics.DrawImage(&bmpOrigen, 0, 0, newW, newH);
				}
				ancho = newW; alto = newH;
				delete[] contenido; contenido = new_content;
			    return *this;
			}
		    img& sesgar(float skewX, float skewY){
			    if(!contenido || ancho <= 0 || alto <= 0) return *reinterpret_cast<img*>(0);
			    skewX = (skewX-360.0f*int(skewX/360.0f)); skewY = (skewY-360.0f*int(skewY/360.0f));
			    if(skewX == 0 && skewY == 0) return *this;
			    // Convertir a Bitmap fuente
			    Gdiplus::Bitmap srcBmp(ancho, alto, ancho * 4, PixelFormat32bppARGB, (BYTE*)contenido);
			    if(srcBmp.GetLastStatus() != Gdiplus::Ok) return *this;
			    // Calcular ángulos en radianes y tangentes
			    constexpr float MAX_TAN = 3.73f; // ≈ tan(75°)
			    constexpr float DEG_TO_RAD = 0.0174532925f; // ≈ pi / 180
			    const float radX = skewX * DEG_TO_RAD;
			    const float radY = skewY * DEG_TO_RAD;
			    const float tanX = __builtin_tanf(radX);
			    const float tanY = __builtin_tanf(radY);
			    if((tanX<0?-tanX:tanX) > MAX_TAN || (tanY<0?-tanY:tanY) > MAX_TAN) return *reinterpret_cast<img*>(0); // Limite aritmetico
			    // Calcular bounding box de la imagen sesgada
			    Gdiplus::PointF corners[] = {
			        {0,            0          },
			        {float(ancho), 0          },
			        {float(ancho), float(alto)},
			        {0,            float(alto)}
			    };
			    Gdiplus::Matrix matrixShear;
			    matrixShear.Shear(tanX, tanY);
			    matrixShear.TransformPoints(corners, 4);
			    float minX = corners[0].X, maxX = corners[0].X;
			    float minY = corners[0].Y, maxY = corners[0].Y;
			    for(int i = 1; i < 4; i++){
			        minX = (minX < corners[i].X ? minX : corners[i].X);
			        maxX = (maxX > corners[i].X ? maxX : corners[i].X);
			        minY = (minY < corners[i].Y ? minY : corners[i].Y);
			        maxY = (maxY > corners[i].Y ? maxY : corners[i].Y);
			    }
			    const int newW = ceil__x(maxX - minX) + 1;
			    const int newH = ceil__x(maxY - minY) + 1;
			    img result(newW, newH);
			    Gdiplus::Bitmap dstBmp(result.ancho, result.alto, result.ancho * 4, PixelFormat32bppARGB, (BYTE*)result.contenido);
			    if(dstBmp.GetLastStatus() != Gdiplus::Ok) *this;
			    {
			        Gdiplus::Graphics g(&dstBmp);
			        g.Clear(Gdiplus::Color(0, 0, 0, 0)); // Fondo transparente
			        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
			        g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
			        g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
			        Gdiplus::Matrix finalTransform;
			        finalTransform.Shear(tanX, tanY);
			        finalTransform.Translate(-minX, -minY, Gdiplus::MatrixOrderAppend); // Compensar desplazamiento
			        g.SetTransform(&finalTransform);
			        g.DrawImage(&srcBmp, 0, 0);
			    }
				(*this) = result; // Actualizar la imagen actual
				delete result.contenido; // Liberar memoria del contenido anterior
			    return *this;
			}
		    img& rotar(float angleDegrees){
				if(!contenido) return *this;
				const int angleMod360 = ((int)round_x(angleDegrees) % 360 + 360) % 360;
				if (angleMod360 == 0) return *this;
				// Rotación exacta sin GDI+
				if(angleMod360 == 90 || angleMod360 == 180 || angleMod360 == 270){
					img result((angleMod360 == 180) ? ancho : alto, (angleMod360 == 180) ? alto : ancho);
					for(int y = 0; y < alto; y++){
						for(int x = 0; x < ancho; x++){
							COLORREF color = at(x, y);
							int nx = 0, ny = 0;
							switch (angleMod360){
								case 90:
									nx = alto - y - 1;
									ny = x;
									break;
								case 180:
									nx = ancho - x - 1;
									ny = alto - y - 1;
									break;
								case 270:
									nx = y;
									ny = ancho - x - 1;
									break;
							}
							result.at(nx, ny) = color;
						}
					}
					(*this) = result; // Actualizar la imagen actual
					delete result.contenido; // Liberar memoria del contenido anterior
					return *this;
				}
				// Rotación general con GDI+
				Gdiplus::Bitmap bmpOrigen(ancho, alto, ancho * 4, PixelFormat32bppARGB, (BYTE*)contenido);
				if(bmpOrigen.GetLastStatus() != Gdiplus::Ok) return *reinterpret_cast<img*>(0);
				const float angleRadians = angleDegrees * (M_PI / 180.f);
				const float sinA = __builtin_sinf(angleRadians);
				const float cosA = __builtin_cosf(angleRadians);
				Gdiplus::PointF corners[4] = {
					{ 0, 0 },
					{ (float)ancho, 0 },
					{ (float)ancho, (float)alto },
					{ 0, (float)alto }
				};
				const float cx = ancho / 2.f, cy = alto / 2.f;
				float minX = corners[0].X, minY = corners[0].Y, maxX = corners[0].X, maxY = corners[0].Y;
				for(int i = 0; i < 4; i++){
					float dx = corners[i].X - cx;
					float dy = corners[i].Y - cy;
					float rx = dx * cosA - dy * sinA;
					float ry = dx * sinA + dy * cosA;
					corners[i].X = rx + cx;
					corners[i].Y = ry + cy;
					minX = (minX < corners[i].X ? minX : corners[i].X);
					maxX = (maxX > corners[i].X ? maxX : corners[i].X);
					minY = (minY < corners[i].Y ? minY : corners[i].Y);
					maxY = (maxY > corners[i].Y ? maxY : corners[i].Y);
				}
				int newW = (int)ceil__x(maxX - minX);
				int newH = (int)ceil__x(maxY - minY);
				img result(newW, newH);
				Gdiplus::Bitmap bmpDestino(newW, newH, newW * 4, PixelFormat32bppARGB, (BYTE*)result.contenido);
				if(bmpDestino.GetLastStatus() != Gdiplus::Ok) return *reinterpret_cast<img*>(0);
				Gdiplus::Graphics g(&bmpDestino);
				g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
				g.SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);
				g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
				g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
				float newCx = newW / 2.f;
				float newCy = newH / 2.f;
				Gdiplus::Matrix m;
				m.RotateAt(angleDegrees, Gdiplus::PointF(newCx, newCy));
				g.SetTransform(&m);
				g.DrawImage(&bmpOrigen, newCx - cx, newCy - cy);
				(*this) = result; // Actualizar la imagen actual
				delete result.contenido; // Liberar memoria del contenido anterior
				return *this;
			}
	};
	class anim{
		float tiempo_acumulado = 0.0f;
		bool im_a_copy = false;
		public:
			unsigned int ancho, alto, frame, cantidad;
			uint32_t** contenido;
			anim() : ancho(0), alto(0), frame(0), cantidad(0), contenido(nullptr), tiempo_acumulado(0.0f), im_a_copy(false) {}
			anim(decltype(nullptr)) : anim() {}
			anim(unsigned int w, unsigned int h) :
				ancho            (w      ),
				alto             (h      ),
				frame            (0      ),
				cantidad         (0      ),
				contenido        (nullptr) {}
			anim(unsigned int w, unsigned int h, unsigned int c) :
				ancho    (w),
				alto     (h),
				frame    (0),
				cantidad (c) {
				contenido = new uint32_t*[cantidad];
				for(int i = 0; i < cantidad; i++) contenido[i] = new uint32_t[w * h];
			}
			anim(const anim& other) :
				ancho            (other.ancho           ),
				alto             (other.alto            ),
				frame            (other.frame           ),
				cantidad         (other.cantidad        ),
				tiempo_acumulado (other.tiempo_acumulado) {
				contenido = new uint32_t*[cantidad];
				for(size_t i = 0, maxItr = ancho * alto; i < cantidad; i++){
				    contenido[i] = new uint32_t[maxItr];
				    for(size_t j=0;j<maxItr;j++)contenido[i][j] = other.contenido[i][j];
				}
			}
			anim(anim&& other) noexcept :
				ancho            (other.ancho           ),
				alto             (other.alto            ),
				frame            (other.frame           ),
				cantidad         (other.cantidad        ),
				contenido        (other.contenido       ),
				tiempo_acumulado (other.tiempo_acumulado) {
				other.ancho = 0;
				other.alto = 0;
				other.frame = 0;
				other.cantidad = 0;
				other.contenido = nullptr;
			}
			anim(const img& _ig, unsigned int count = 0U) : ancho(_ig.ancho), alto(_ig.alto), frame(0), tiempo_acumulado(0.0f){
				cantidad = count + 1; contenido = new uint32_t*[cantidad];
				for(unsigned int i = 0; i <= count; i++){
					contenido[i] = new uint32_t[ancho * alto];
					for(size_t j=0,m=ancho * alto;j<m;j++)contenido[i][j]=_ig.contenido[j];
				}
			}
			anim(unsigned int w, unsigned int h, uint32_t* cc, unsigned int c) :
				ancho     (w               ),
				alto      (h               ),
				frame     (0               ),
				cantidad  (c               ),
				im_a_copy (true            ),
				contenido (new uint32_t*[c]) { for(size_t i = 0; i < c; i++) contenido[i] = cc; }
			anim(unsigned int w, unsigned int h, uint32_t** cc, unsigned int c) :
				ancho     (w   ),
				alto      (h   ),
				frame     (0   ),
				cantidad  (c   ),
				im_a_copy (true),
				contenido (cc  ) {}
			void limpiar_frames(){
			    if(contenido){ for(unsigned int i = 0; !im_a_copy && i < cantidad; ++i) if(contenido[i]) delete[] contenido[i]; delete[] contenido; }
			    contenido = nullptr;
			    cantidad = frame = ancho = alto = 0;
			}
			~anim(){ limpiar_frames(); }
			img operator[](unsigned int _n) const{
				if(!contenido || cantidad == 0 || _n >= cantidad) return {};
				return {ancho, alto, contenido[_n]};
			}
			img obtener_frame() const{ return (*this)[frame]; }
			anim& operator=(const img& other){
				limpiar_frames();
				cantidad = 1; contenido = new uint32_t*[cantidad];
				ancho = other.ancho; alto = other.alto;
				contenido[0] = new uint32_t[ancho * alto];
				for(size_t i=0,j=ancho * alto;i<j;i++)contenido[0][i]=other.contenido[i];
				return *this;
			}
			anim& operator=(img&& other) noexcept{
				limpiar_frames();
				ancho = other.ancho;
				alto = other.alto;
				cantidad = 1;
				frame = 0;
				contenido = new uint32_t*[1];
				contenido[0] = other.contenido;
				other.contenido = nullptr;
				other.ancho = other.alto = 0;
				return *this;
			}
			anim& operator=(const anim& other){
			    if(this == &other) return *this;
				limpiar_frames();
			    if(other.contenido == nullptr || other.cantidad == 0) return *this;
				ancho = other.ancho;
				alto = other.alto;
				cantidad = other.cantidad;
				frame = other.frame;
				contenido = new uint32_t*[cantidad];
				for(size_t i = 0; i < cantidad; i++){
				    contenido[i] = new uint32_t[ancho * alto]{0};
				    if(other.contenido[i]) for(size_t j=0,m=ancho * alto;j<m;j++)contenido[i][j]=other.contenido[i][j];
				}
				return *this;
			}
			anim& operator=(anim&& other) noexcept{
			    if(this == &other) return *this;
				limpiar_frames();
				ancho = other.ancho;
				alto = other.alto;
				frame = other.frame;
				cantidad = other.cantidad;
				contenido = other.contenido;
				other.ancho = other.alto = other.frame = other.cantidad = 0;
				other.contenido = nullptr;
				return *this;
			}
			anim& operator+=(const img& other){
				if(!other.valido()) return *this;
			    if(!valido()){
			        *this = other;
			        return *this;
			    }
			    if(cantidad == 0 || ancho == 0 || alto == 0){
			        ancho = other.ancho;
			        alto  = other.alto;
			    }
			    if(ancho != other.ancho || alto != other.alto) return *this;
			    uint32_t** new_dst = new uint32_t*[cantidad + 1];
			    for(size_t i = 0; i < cantidad; i++) new_dst[i] = contenido[i];
			    new_dst[cantidad] = new uint32_t[ancho * alto];
			    for(size_t j=0,m=ancho * alto;j<m;j++)contenido[cantidad][j]=other.contenido[j];
				contenido = new_dst;
				cantidad++;
			    return *this;
			}
			anim& operator+=(const anim& other){
			    if(!valido()) return (*this = other);
				if(!other.valido()) return *this;
			    if(ancho != other.ancho || alto != other.alto) return *this;
			    uint32_t** resultado = new uint32_t*[cantidad + other.cantidad];
				for(size_t i = 0; i < cantidad; i++) resultado[i] = contenido[i];
				for(size_t i = cantidad; i < cantidad + other.cantidad; i++){
					resultado[i] = new uint32_t[ancho * alto];
					for(size_t j=0,m=ancho*alto,n=i-cantidad;j<m;j++)resultado[i][j]=other.contenido[n][j];
				}
				delete[] contenido; contenido = resultado;
				cantidad += other.cantidad;
			    return *this;
			}
			anim operator+(const img& other) const{
				anim result = *this;
				result += other;
				return result;
			}
			anim operator+(const anim& other) const{
				anim result = *this;
				result += other;
				return result;
			}
			bool operator==(const img& other) const{
				if(cantidad != 1 || !contenido || !other.contenido) return !(!contenido ^ !other.contenido);
				if(ancho != other.ancho || ancho != other.ancho) return false;
				for(size_t i=0,j=ancho * alto;i<j;i++)if(contenido[0][i]!=other.contenido[i])return false;
				return true;
			}
			bool operator==(const anim& other) const{
				if(!contenido || !other.contenido) return !(!contenido ^ !other.contenido);
				if(ancho != other.ancho || ancho != other.ancho || cantidad != other.cantidad) return false;
				for(size_t i = 0; i < cantidad; i++)
					for(size_t j=0,m=ancho * alto;j<m;j++)if(contenido[i][j]!=other.contenido[i][j])return false;
				return true;
			}
			bool operator!=(const img& other)  const{ return !((*this) == other); }
			bool operator!=(const anim& other) const{ return !((*this) == other); }
			bool valido() const{ return (contenido != nullptr && ancho != 0 && alto != 0 && cantidad != 0); }
			void agregar_frame(const img& other){ *this += other; }
			void agregar_frame(const img& other, const unsigned int& time){ *this += anim(other.ancho, other.alto, other.contenido, time); }
			void agregar_frame(const anim& other){ *this += other; }
			void eliminar_frame(size_t _x, size_t _n = 1){
			    if(_x >= (size_t)cantidad || _n == 0) return;
			    if(_x + _n > (size_t)cantidad) _n = cantidad - _x;
			    if(cantidad - _n == 0){ *this = anim(); return; }
			    uint32_t** new_contenido = new uint32_t*[cantidad - _n];
			    size_t j = 0;
				for(size_t i =      0; i <       _x; i++) new_contenido[j++] = contenido[i];
				for(size_t i = _x + _n; i < cantidad; i++) new_contenido[j++] = contenido[i];
			    delete[] contenido; contenido = new_contenido;
			    cantidad -= _n;
			    frame %= cantidad;
			}
			      uint32_t& at(size_t n, int x, int y)      { return contenido[n % cantidad][y * ancho + x]; }
			const uint32_t& at(size_t n, int x, int y) const{ return contenido[n % cantidad][y * ancho + x]; }
			anim segmento(unsigned int desde = 0U, unsigned int _con = 0U) const{
				if(!contenido || _con == 0 || desde >= cantidad) return {};
				if(_con == 0 || (desde + _con) > cantidad) _con = cantidad - desde;
				uint32_t** result = new uint32_t*[_con];
				for(size_t i = 0; i < _con; i++) result[i] = contenido[desde + i];
				return {ancho, alto, result, _con};
			}
			anim& siguiente_frame(unsigned int _n = 0U){
				if(cantidad == 0) return *this;
				frame++; frame += _n;
				while(frame >= cantidad) frame -= cantidad;
				return *this;
			}
			anim& anterior_frame(unsigned int _n = 0U){
				if(cantidad == 0) return *this;
				frame--; frame -= _n;
				while(frame < 0) frame += cantidad;
				return *this;
			}
			anim& estirar(int w, int h){
				if(w == 0 || h == 0 || !contenido) return *this;
			    if(w == ancho && h == alto) return *this;
				const unsigned int newW = (w < 0) ? -w : w;
				const unsigned int newH = (h < 0) ? -h : h;
			    uint32_t** new_content = new uint32_t*[cantidad];
			    for(size_t i=0; i<cantidad; i++){
				    Gdiplus::Bitmap bmpOrigen(ancho, alto, ancho * 4, PixelFormat32bppARGB, (BYTE*)contenido[i]);
				    if(bmpOrigen.GetLastStatus() != Gdiplus::Ok){
				    	for(size_t j=i; j>=0; j--) if(new_content[i]) delete[] new_content[i];
				    	delete[] new_content;
				    	continue;
					}
					new_content[i] = new uint32_t[newW * newH];
					Gdiplus::Bitmap bmpDestino(newW, newH, newW * 4, PixelFormat32bppARGB, (BYTE*)new_content[i]);
					if(bmpDestino.GetLastStatus() != Gdiplus::Ok){
						for(size_t j=i; j>=0; j--) if(new_content[i]) delete[] new_content[i];
				    	delete[] new_content;
						continue;
					}
					{
						Gdiplus::Graphics graphics(&bmpDestino);
						graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
						graphics.SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);
						graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
						if(w < 0 || h < 0){ // Preparar transformación si hay volteo
							Gdiplus::Matrix transform;
							transform.Translate(w < 0 ? float(newW) : 0, h < 0 ? float(newH) : 0);
							transform.Scale(w < 0 ? -1.0f : 1.0f, h < 0 ? -1.0f : 1.0f);
							graphics.SetTransform(&transform);
						}
						graphics.DrawImage(&bmpOrigen, 0, 0, newW, newH);
					}
					delete[] contenido[i];
				}
				ancho = newW; alto = newH;
				delete[] contenido; contenido = new_content;
			    return *this;
			}
	};
	////// AUXILIAR TO MDL //////
	void compress_dot(uint64_t* result, const pnt& value){
		int16_t ix = value.x,
				iy = value.y;
		uint64_t fx = ((uint64_t(ix & 0x8000) << 5) | (ix & 0x7FFF));
		uint64_t fy = ((uint64_t(iy & 0x8000) >> 5) | (iy & 0x7FFF));
		*result = (fx << 43) | (fy << 22) | uint16_t(value.z);
	}
	void descompress_dot(const uint64_t data, pnt* result){
		uint32_t bits_x = (data >> 43) & 0x3FFFFF,
				 bits_y = (data >> 22) & 0x3FFFFF,
				 bits_z =  data        & 0x3FFFFF;
		result->x = int32_t(((bits_x & 0x100000) << 11) | (bits_x & 0xFFFFF));
		result->y = int32_t(((bits_y & 0x100000) << 11) | (bits_y & 0xFFFFF));
		result->z = bits_z;
	}
	////////////////////////////
	class mdl{
		size_t vert_size = 0;
		pnt*   vert_list = nullptr;
		size_t  faces_capacity = 0;
		uint8_t* faces_sizes   = nullptr;
		size_t* faces_list     = nullptr;
		uint64_t* uvs_list     = nullptr;
		public:
			anim textura; pnt pivote;
			bool brillo = true, suavizado = false;
			// Funciones
			bool textura_valida() const{ return textura.valido(); }
			bool textura_valida(size_t _x) const{
				if(!textura.valido() || _x >= textura.cantidad) return false;
				return (textura.contenido[_x] && textura.ancho != 0 && textura.alto != 0);
			}
			const pnt* datos_vertices() const{ return vert_list; }
			      pnt* datos_vertices()      { return vert_list; }
			const uint8_t* datos_longitudes_faces() const{ return faces_sizes; }
			      uint8_t* datos_longitudes_faces()      { return faces_sizes; }
			const size_t* datos_faces() const{ return faces_list; }
			      size_t* datos_faces()      { return faces_list; }
			const uint64_t* datos_uvs() const{ return uvs_list; }
			      uint64_t* datos_uvs()      { return uvs_list; }
			void agregar_vertice(const pnt& v){
				pnt* _new_vert_list = new pnt[vert_size + 1];
				for(size_t i = 0; i < vert_size; i++) _new_vert_list[i] = vert_list[i];
				_new_vert_list[vert_size] = v;
				if(vert_list) delete[] vert_list; vert_list = _new_vert_list;
				vert_size++;
			}
			void agregar_vertice(const float& _x, const float& _y, const float& _z){
				pnt* _new_vert_list = new pnt[vert_size + 1];
				for(size_t i = 0; i < vert_size; i++) _new_vert_list[i] = vert_list[i];
				_new_vert_list[vert_size] = {_x,_y,_z};
				if(vert_list) delete[] vert_list; vert_list = _new_vert_list;
				vert_size++;
			}
			size_t cantidad_vertices() const{ return vert_size; }
			pnt obtener_vertice(size_t _x) const{
				if(!vert_list || vert_size == 0) return {};
				return vert_list[_x];
			}
			void modificar_vertice(size_t _x, const pnt& _new_dot){
				if(!vert_list || vert_size == 0) return;
				vert_list[_x] = _new_dot;
			}
		    bool eliminar_vertice(size_t _x, size_t _n = 1){
				if(_x >= vert_size || _n == 0) return false;
				if(_x + _n > vert_size) _n = vert_size - _x;
				pnt* _nuevo = new pnt[vert_size - _n];
				for(size_t i = 0, j = 0; i < vert_size; ++i) if(i < _x || i >= _x + _n) _nuevo[j++] = vert_list[i];
				if(vert_list) delete[] vert_list; vert_list = _nuevo;
				vert_size -= _n;
				return true;
			}
		    bool limpiar_vertices(){
				if(vert_size == 0 || !vert_list) return false;
				delete[] vert_list;
				vert_list = nullptr;
				vert_size = 0;
				return true;
			}
			void agregar_cara(size_t i1, size_t i2, pnt tp1 = {0,0}, pnt tp2 = {0,0}){
				size_t summon = 2;
				uint8_t* _new_faces_sizes = new uint8_t[faces_capacity + 1];
				size_t total_size = 0; for(size_t i = 0; i < faces_capacity; i++){
					total_size += faces_sizes[i];
					_new_faces_sizes[i] = (faces_sizes[i]);
				}
				_new_faces_sizes[faces_capacity] = summon;
				faces_sizes = _new_faces_sizes;
				size_t* _new_faces_list = new size_t[total_size + summon];
				uint64_t* _new_uvs_list = new uint64_t[total_size + summon];
				for(size_t i = 0; i < total_size; i++){
					_new_faces_list[i] = (faces_list[i]);
					_new_uvs_list[i]   = (uvs_list[i]);
				}
				_new_faces_list[total_size]     = i1;
				_new_faces_list[total_size + 1] = i2;
				compress_dot(_new_uvs_list + total_size,     tp1);
				compress_dot(_new_uvs_list + total_size + 1, tp2);
				faces_list = _new_faces_list;
				uvs_list   = _new_uvs_list;
				faces_capacity++;
			}
			void agregar_cara(size_t i1, size_t i2, size_t i3, pnt tp1 = pnt(0, 0), pnt tp2 = pnt(0, 0), pnt tp3 = pnt(0, 0)){
				size_t summon = 3;
				uint8_t* _new_faces_sizes = new uint8_t[faces_capacity + 1];
				size_t total_size = 0; for(size_t i = 0; i < faces_capacity; i++){
					total_size += faces_sizes[i];
					_new_faces_sizes[i] = (faces_sizes[i]);
				}
				_new_faces_sizes[faces_capacity] = summon;
				faces_sizes = _new_faces_sizes;
				size_t* _new_faces_list = new size_t[total_size + summon];
				uint64_t* _new_uvs_list = new uint64_t[total_size + summon];
				for(size_t i = 0; i < total_size; i++){
					_new_faces_list[i] = (faces_list[i]);
					_new_uvs_list[i]   = (uvs_list[i]);
				}
				_new_faces_list[total_size    ] = i1;
				_new_faces_list[total_size + 1] = i2;
				_new_faces_list[total_size + 2] = i3;
				compress_dot(_new_uvs_list + total_size,     tp1);
				compress_dot(_new_uvs_list + total_size + 1, tp2);
				compress_dot(_new_uvs_list + total_size + 2, tp3);
				faces_list = _new_faces_list;
				uvs_list = _new_uvs_list;
				faces_capacity++;
			}
			void agregar_cara(size_t i1, size_t i2, size_t i3, size_t i4, pnt tp1 = (0, 0), pnt tp2 = pnt(0, 0), pnt tp3 = pnt(0, 0), pnt tp4 = pnt(0, 0)){
				size_t summon = 4;
				uint8_t* _new_faces_sizes = new uint8_t[faces_capacity + 1];
				size_t total_size = 0; for(size_t i = 0; i < faces_capacity; i++){
					total_size += faces_sizes[i];
					_new_faces_sizes[i] = (faces_sizes[i]);
				}
				_new_faces_sizes[faces_capacity] = summon;
				faces_sizes = _new_faces_sizes;
				size_t* _new_faces_list = new size_t[total_size + summon];
				uint64_t* _new_uvs_list = new uint64_t[total_size + summon];
				for(size_t i = 0; i < total_size; i++){
					_new_faces_list[i] = (faces_list[i]);
					_new_uvs_list[i]   = (uvs_list[i]);
				}
				_new_faces_list[total_size    ] = i1;
				_new_faces_list[total_size + 1] = i2;
				_new_faces_list[total_size + 2] = i3;
				_new_faces_list[total_size + 3] = i4;
				compress_dot(_new_uvs_list + total_size,     tp1);
				compress_dot(_new_uvs_list + total_size + 1, tp2);
				compress_dot(_new_uvs_list + total_size + 2, tp3);
				compress_dot(_new_uvs_list + total_size + 3, tp4);
				faces_list = _new_faces_list;
				uvs_list = _new_uvs_list;
				faces_capacity++;
			}
			size_t cantidad_caras() const{ return faces_capacity; }
			size_t longitud_cara(size_t _x) const{
				if(!faces_sizes || faces_capacity == 0) return -1;
				return faces_sizes[_x];
			}
			size_t obtener_cara(size_t _face, int _dot) const{
				if(!faces_sizes || !faces_list || faces_capacity == 0) return -1;
				size_t pos = 0, i = 0; while(i < _face) pos += faces_sizes[i++];
				return faces_list[pos + __clamp<size_t>(_dot, 0, faces_sizes[i] - 1)];
			}
			pnt obtener_uv(size_t _face, int _uv) const{
				if(!faces_sizes || !uvs_list || faces_capacity == 0) return {};
				size_t pos = 0, i = 0; while(i < _face) pos += faces_sizes[i++];
				pnt result; descompress_dot(uvs_list[pos + __clamp<size_t>(_uv, 0, faces_sizes[i] - 1)], &result);
				return result;
			}
			void modificar_cara(size_t _face, size_t i1, size_t i2){
				if(!faces_sizes || !faces_list || !uvs_list || faces_capacity == 0) return;
				size_t total_size = 0; for(size_t i = 0; i < faces_capacity; i++) total_size += faces_sizes[i];
				size_t* _new_faces_list = new size_t[total_size - faces_sizes[_face] + 2];
				uint64_t* _new_uvs_list = new uint64_t[total_size - faces_sizes[_face] + 2];
				for(size_t i = 0, j = 0, k = 0; i < faces_capacity; i++){
					if(i == _face){
						_new_faces_list[k] = i1;
						_new_faces_list[k + 1] = i2;
						_new_uvs_list[k] = (uvs_list[j]);
						_new_uvs_list[k + 1] = (uvs_list[j + 1]);
						k += 2; j += faces_sizes[_face]; continue;
					}
					size_t l = faces_sizes[i]; while(l--){
						_new_faces_list[k] = (faces_list[j]);
						_new_uvs_list[k]   =   (uvs_list[j]);
						k++; j++;
					}
				}
				delete[] faces_list; faces_list = _new_faces_list;
				delete[] uvs_list;   uvs_list   = _new_uvs_list;
				faces_sizes[_face] = 2;
			}
			void modificar_cara(size_t _face, size_t i1, size_t i2, size_t i3){
				if(!faces_sizes || !faces_list || !uvs_list || faces_capacity == 0) return;
				size_t total_size = 0; for(size_t i = 0; i < faces_capacity; i++) total_size += faces_sizes[i];
				size_t* _new_faces_list = new size_t[total_size - faces_sizes[_face] + 3];
				uint64_t* _new_uvs_list = new uint64_t[total_size - faces_sizes[_face] + 3];
				for(size_t i = 0, j = 0, k = 0; i < faces_capacity; i++){
					if(i == _face){
						_new_faces_list[k] = i1;
						_new_faces_list[k + 1] = i2;
						_new_faces_list[k + 2] = i3;
						_new_uvs_list[k] = (uvs_list[j]);
						_new_uvs_list[k + 1] = uvs_list[j + 1];
						_new_uvs_list[k + 2] = (faces_sizes[_face] >= 3) ? (uvs_list[j + 2]) : 0x0ULL;
						k += 3; j += faces_sizes[_face]; continue;
					}
					size_t l = faces_sizes[i]; while(l--){
						_new_faces_list[k] = (faces_list[j]);
						_new_uvs_list[k]   =   (uvs_list[j]);
						k++; j++;
					}
				}
				delete[] faces_list; faces_list = _new_faces_list;
				delete[] uvs_list;   uvs_list   = _new_uvs_list;
				faces_sizes[_face] = 3;
			}
			void modificar_cara(size_t _face, size_t i1, size_t i2, size_t i3, size_t i4){
				if(!faces_sizes || !faces_list || !uvs_list || faces_capacity == 0) return;
				size_t total_size = 0; for(size_t i = 0; i < faces_capacity; i++) total_size += faces_sizes[i];
				size_t* _new_faces_list = new size_t[total_size - faces_sizes[_face] + 4];
				uint64_t* _new_uvs_list = new uint64_t[total_size - faces_sizes[_face] + 4];
				for(size_t i = 0, j = 0, k = 0; i < faces_capacity; i++){
					if(i == _face){
						_new_faces_list[k] = i1;
						_new_faces_list[k + 1] = i2;
						_new_faces_list[k + 2] = i3;
						_new_faces_list[k + 3] = i4;
						_new_uvs_list[k] = (uvs_list[j]);
						_new_uvs_list[k + 1] = (uvs_list[j + 1]);
						_new_uvs_list[k + 2] = (faces_sizes[_face] >= 3) ? (uvs_list[j + 2]) : 0x0ULL;
						_new_uvs_list[k + 3] = (faces_sizes[_face] == 4) ? (uvs_list[j + 3]) : 0x0ULL;
						k += 4; j += faces_sizes[_face]; continue;
					}
					size_t l = faces_sizes[i]; while(l--){
						_new_faces_list[k] = (faces_list[j]);
						_new_uvs_list[k]   =   (uvs_list[j]);
						k++; j++;
					}
				}
				delete[] faces_list; faces_list = _new_faces_list;
				delete[] uvs_list;   uvs_list   = _new_uvs_list;
				faces_sizes[_face] = 4;
			}
			void modificar_uv(size_t _face, pnt _new_tp1, pnt _new_tp2){
				if(!faces_sizes || !faces_list || !uvs_list || faces_capacity == 0) return;
				size_t total_size = 0; for(size_t i = 0; i < faces_capacity; i++) total_size += faces_sizes[i];
				size_t* _new_faces_list = new size_t[total_size - faces_sizes[_face] + 2];
				uint64_t* _new_uvs_list = new uint64_t[total_size - faces_sizes[_face] + 2];
				for(size_t i = 0, j = 0, k = 0; i < faces_capacity; i++){
					if(i == _face){
						_new_faces_list[k]     =     (faces_list[j]);
						_new_faces_list[k + 1] = (faces_list[j + 1]);
						compress_dot(_new_uvs_list + k,     _new_tp1);
						compress_dot(_new_uvs_list + k + 1, _new_tp2);
						k += 2; j += faces_sizes[_face]; continue;
					}
					size_t l = faces_sizes[i]; while(l--){
						_new_faces_list[k] = (faces_list[j]);
						_new_uvs_list[k]   =   (uvs_list[j]);
						k++; j++;
					}
				}
				delete[] faces_list; faces_list = _new_faces_list;
				delete[] uvs_list;   uvs_list   = _new_uvs_list;
				faces_sizes[_face] = 2;
			}
			void modificar_uv(size_t _face, pnt _new_tp1, pnt _new_tp2, pnt _new_tp3){
				if(!faces_sizes || !faces_list || !uvs_list || faces_capacity == 0) return;
				size_t total_size = 0; for(size_t i = 0; i < faces_capacity; i++) total_size += faces_sizes[i];
				size_t* _new_faces_list = new size_t[total_size - faces_sizes[_face] + 3];
				uint64_t* _new_uvs_list = new uint64_t[total_size - faces_sizes[_face] + 3];
				for(size_t i = 0, j = 0, k = 0; i < faces_capacity; i++){
					if(i == _face){
						_new_faces_list[k]     =     (faces_list[j]);
						_new_faces_list[k + 1] = (faces_list[j + 1]);
						_new_faces_list[k + 2] = (faces_sizes[_face] >= 3) ? (faces_list[j + 2]) : 0x0ULL;
						compress_dot(_new_uvs_list + k,     _new_tp1);
						compress_dot(_new_uvs_list + k + 1, _new_tp2);
						compress_dot(_new_uvs_list + k + 2, _new_tp3);
						k += 3; j += faces_sizes[_face]; continue;
					}
					size_t l = faces_sizes[i]; while(l--){
						_new_faces_list[k] = (faces_list[j]);
						_new_uvs_list[k]   =   (uvs_list[j]);
						k++; j++;
					}
				}
				delete[] faces_list; faces_list = _new_faces_list;
				delete[] uvs_list;   uvs_list   = _new_uvs_list;
				faces_sizes[_face] = 3;
			}
			void modificar_uv(size_t _face, pnt _new_tp1, pnt _new_tp2, pnt _new_tp3, pnt _new_tp4){
				if(!faces_sizes || !faces_list || !uvs_list || faces_capacity == 0) return;
				size_t total_size = 0; for(size_t i = 0; i < faces_capacity; i++) total_size += faces_sizes[i];
				size_t* _new_faces_list = new size_t[total_size - faces_sizes[_face] + 4];
				uint64_t* _new_uvs_list = new uint64_t[total_size - faces_sizes[_face] + 4];
				for(size_t i = 0, j = 0, k = 0; i < faces_capacity; i++){
					if(i == _face){
						_new_faces_list[k]     =     (faces_list[j]);
						_new_faces_list[k + 1] = (faces_list[j + 1]);
						_new_faces_list[k + 2] = (faces_sizes[_face] >= 3) ? (faces_list[j + 2]) : 0x0ULL;
						_new_faces_list[k + 3] = (faces_sizes[_face] == 4) ? (faces_list[j + 3]) : 0x0ULL;
						compress_dot(_new_uvs_list + k,     _new_tp1);
						compress_dot(_new_uvs_list + k + 1, _new_tp2);
						compress_dot(_new_uvs_list + k + 2, _new_tp3);
						compress_dot(_new_uvs_list + k + 3, _new_tp4);
						k += 4; j += faces_sizes[_face]; continue;
					}
					size_t l = faces_sizes[i]; while(l--){
						_new_faces_list[k] = (faces_list[j]);
						_new_uvs_list[k]   =   (uvs_list[j]);
						k++; j++;
					}
				}
				delete[] faces_list; faces_list = _new_faces_list;
				delete[] uvs_list;   uvs_list   = _new_uvs_list;
				faces_sizes[_face] = 4;
			}
			bool eliminar_cara(size_t _x, size_t _n = 1){
				if(_x >= faces_capacity || _n == 0) return false;
				if(_x + _n > faces_capacity) _n = faces_capacity - _x;
				uint8_t* _new_faces_sizes = new uint8_t[faces_capacity - _n];
				size_t _new_total_size = 0; for(size_t i = 0, j = 0; i < faces_capacity; i++) if(i < _x || i >= _x + _n){
					_new_faces_sizes[j++] = faces_sizes[i];
					_new_total_size += faces_sizes[i];
				}
				size_t* _new_faces_list = new size_t[_new_total_size];
				uint64_t* _new_uvs_list = new uint64_t[_new_total_size];
				for(size_t i = 0, j = 0, k = 0; i < faces_capacity; i++){
					if(i >= _x && i < _x + _n){ // Saltamos la cara que debe eliminarse
						j += faces_sizes[i];
						continue;
					}
					size_t face_size = faces_sizes[i]; while(face_size--){
						_new_faces_list[k] = faces_list[j];
						_new_uvs_list[k++] = uvs_list[j++];
					}
				}
				delete[] faces_sizes; faces_sizes = _new_faces_sizes;
				delete[] faces_list;  faces_list  = _new_faces_list;
				delete[] uvs_list;    uvs_list    = _new_uvs_list;
				faces_capacity -= _n;
				return true;
			}
			bool limpiar_caras(){
				if(faces_capacity == 0 || !faces_sizes || !faces_list || !uvs_list) return false;
				if(faces_sizes) delete[] faces_sizes; faces_sizes = nullptr;
				if(faces_list)  delete[] faces_list;  faces_list  = nullptr;
				if(uvs_list)    delete[] uvs_list;    uvs_list    = nullptr;
				faces_capacity = 0;
				return true;
			}
		    pnt normal_de(size_t _x) const{
				if(!vert_list || vert_size == 0 || !faces_sizes || !faces_list || faces_capacity == 0 || _x >= faces_capacity) return {};
				size_t pos = 0, i = 0; while(i < _x) pos += faces_sizes[i++];
				switch(faces_sizes[_x]){
					case 2:{
						pnt va = vert_list[faces_list[pos    ]] - pivote,
							vb = vert_list[faces_list[pos + 1]] - pivote;
						return (vb - va).normalizar();
					break;}
					case 3:{
						pnt va = vert_list[faces_list[pos    ]] - pivote,
							vb = vert_list[faces_list[pos + 1]] - pivote,
							vc = vert_list[faces_list[pos + 2]] - pivote;
						return (vb - va).cruz(vc - va).normalizar();
					break;}
					case 4:{
						pnt va = vert_list[faces_list[pos    ]] - pivote,
							vb = vert_list[faces_list[pos + 1]] - pivote,
							vc = vert_list[faces_list[pos + 2]] - pivote,
							vd = vert_list[faces_list[pos + 3]] - pivote;
						return (((vb - va).cruz(vc - va) + (vc - va).cruz(vd - va)) * 0.5f).normalizar();
					break;}
				}
				return {};
			}
		    void rellenar_textura(){
				if(!faces_sizes || !uvs_list || faces_capacity == 0) return;
				const bool is_valida = textura.valido();
				// Si la textura es válida, se usarán sus dimensiones. Si no, todo será 0.
				const float w = is_valida ? float(textura.ancho) : 0.0f;
				const float h = is_valida ? float(textura.alto)  : 0.0f;
				// Coordenadas UV para los 4 puntos base del quad (sentido antihorario)
				const float pointers[4][2] = {
					{0.0f, 0.0f},
					{w,    0.0f},
					{w,    h},
					{0.0f, h}
				};
				for(size_t i = 0, j = 0; i < faces_capacity; i++){
					const size_t verts = faces_sizes[i];
					for(size_t k = 0; k < verts; ++k){
						// Índice base para cuadrado: 0,1,2,3
						size_t l = k;
						// Si es triángulo (3 vértices), repite el primer punto en el último vértice
						if(verts == 3 && k == verts - 1) l = 0;
						pnt aux; descompress_dot(uvs_list[j], &aux);
						if(is_valida) compress_dot(uvs_list + j, pnt{ pointers[l][0], pointers[l][1], aux.z });
						else          uvs_list[j] = 0x0ULL;
						++j;
					}
				}
			}
		    mdl& trasladar(const pnt& delta){ return (*this += delta); }
		    mdl& escalar(float factor){ return (*this *= factor); }
		    mdl& rotarX(float rad){
				float cosA =  __builtin_cosf(rad), sinA = __builtin_sinf(rad);
				for(size_t i = 0; i < vert_size; i++) vert_list[i] = pnt{
					vert_list[i].x - pivote.x,
					(vert_list[i].y - pivote.y) * cosA - (vert_list[i].z - pivote.z) * sinA,
					(vert_list[i].y - pivote.y) * sinA + (vert_list[i].z - pivote.z) * cosA
				} + pivote;
				return *this;
			}
			mdl& rotarY(float rad){
				float cosA =  __builtin_cosf(rad), sinA = __builtin_sinf(rad);
				for(size_t i = 0; i < vert_size; i++) vert_list[i] = pnt{
					(vert_list[i].z - pivote.z) * sinA + (vert_list[i].x - pivote.x) * cosA,
					vert_list[i].y - pivote.y,
					(vert_list[i].z - pivote.z) * cosA - (vert_list[i].x - pivote.x) * sinA
				} + pivote;
				return *this;
			}
			mdl& rotarZ(float rad){
				float cosA =  __builtin_cosf(rad), sinA = __builtin_sinf(rad);
				for(size_t i = 0; i < vert_size; i++) vert_list[i] = pnt{
					(vert_list[i].x - pivote.x) * cosA - (vert_list[i].y - pivote.y) * sinA,
					(vert_list[i].x - pivote.x) * sinA + (vert_list[i].y - pivote.y) * cosA,
					vert_list[i].z - pivote.z
				} + pivote;
				return *this;
			}
		    // Operadores
			mdl& operator=(const mdl& o){
				if(this == &o) return *this;
				limpiar_vertices();
				limpiar_caras();
				// Copiar Vertices
				vert_size = o.vert_size;
				vert_list = new pnt[vert_size];
				for(size_t i=0;i<vert_size;i++)vert_list[i]=o.vert_list[i];
				// Copiar Caras y UVs
				faces_capacity = o.faces_capacity;
				faces_sizes = new uint8_t[faces_capacity]; for(size_t i=0;i<faces_capacity;i++)faces_sizes[i]=o.faces_sizes[i];
				size_t total_size = 0; for(size_t i = 0; i < faces_capacity; i++) total_size += faces_sizes[i];
				faces_list = new size_t[total_size];   for(size_t i=0;i<total_size;i++)faces_list[i]=o.faces_list[i];
				uvs_list   = new uint64_t[total_size]; for(size_t i=0;i<total_size;i++)uvs_list[i]=o.uvs_list[i];
				// Copiar metadata
				textura   = o.textura;
				pivote    = o.pivote;
				brillo    = o.brillo;
				suavizado = o.suavizado;
				return *this;
			}
			mdl& operator=(mdl&& o) noexcept{
				if(this == &o) return *this;
				// Liberar recursos actuales
				limpiar_vertices();
				limpiar_caras();
				// Encadenar datos del objeto fuente al objeto actual
				vert_size = o.vert_size;
				vert_list = o.vert_list;
				faces_capacity = o.faces_capacity;
				faces_sizes    = o.faces_sizes;
				faces_list     = o.faces_list;
				uvs_list       = o.uvs_list;
				textura   = o.textura;
				pivote    = o.pivote;
				brillo    = o.brillo;
				suavizado = o.suavizado;
				// Desencadenar datos del objeto fuente
				o.vert_size = 0; o.vert_list = nullptr;
				o.faces_capacity = 0;
				o.faces_sizes = nullptr; o.faces_list = nullptr;
				o.uvs_list = nullptr;
				o.brillo = o.suavizado = false;
				return *this;
			}
		    bool operator==(const mdl& o) const{
				if(vert_size      != o.vert_size ||
				   faces_capacity != o.faces_capacity) return false;
				for(size_t i=0;i<faces_capacity;i++)if(faces_sizes[i]!=o.faces_sizes[i])return false;
				size_t total_size = 0; for(size_t i = 0; i < faces_capacity; i++) total_size += faces_sizes[i];
				for(size_t i=0;i<vert_size;i++)if(vert_list[i]!=o.vert_list[i])return false;
				for(size_t i=0;i<total_size;i++){
					if(faces_list[i]!=o.faces_list[i])return false;
					if(uvs_list[i]!=o.uvs_list[i])return false;
				}
				return textura == o.textura && pivote == o.pivote &&
					   !(brillo ^ o.brillo) && !(suavizado ^ o.suavizado);
			} bool operator!=(const mdl& o) const{ return !(*this == o); }
		    mdl& operator+=(const mdl& o){
				pnt* _new_vert_list = new pnt[vert_size + o.vert_size];
				for(size_t i = 0; i < vert_size; i++) _new_vert_list[i] = vert_list[i];
				for(size_t i = vert_size; i - vert_size < o.vert_size; i++) _new_vert_list[i] = o.vert_list[i - vert_size];
				if(vert_list) delete[] vert_list; vert_list = _new_vert_list;
				uint8_t* _new_faces_sizes = new uint8_t[faces_capacity + o.faces_capacity];
				size_t total_size = 0, _other_total_size = 0; for(size_t i = 0; i < faces_capacity; i++){
					total_size += faces_sizes[i];
					_new_faces_sizes[i] = faces_sizes[i];
				} for(size_t i = faces_capacity; i - faces_capacity < o.faces_capacity; i++){
					_other_total_size += o.faces_sizes[i - faces_capacity];
					_new_faces_sizes[i] = o.faces_sizes[i - faces_capacity];
				}
				if(faces_sizes) delete[] faces_sizes; faces_sizes = _new_faces_sizes;
				size_t* _new_faces_list = new size_t[total_size + _other_total_size];
				uint64_t* _new_uvs_list = new uint64_t[total_size + _other_total_size];
				for(size_t i = 0; i < total_size; i++){
					_new_faces_list[i] = faces_list[i];
					_new_uvs_list[i]   = uvs_list[i];
				} for(size_t i = total_size; i - total_size < _other_total_size; i++){
					_new_faces_list[i] = o.faces_list[i - total_size] + vert_size;
					_new_uvs_list[i]   =   o.uvs_list[i - total_size];
				}
				if(faces_list) delete[] faces_list; faces_list = _new_faces_list;
				if(uvs_list) delete[] uvs_list; uvs_list   =   _new_uvs_list;
				vert_size += o.vert_size; faces_capacity += o.faces_capacity;
				return *this;
			} mdl operator+(const mdl& o) const{ mdl r = *this; r += o; return r; }
		    mdl& operator+=(const pnt& p){
				for(size_t i = 0; i < vert_size; i++) vert_list[i] += p; pivote += p;
				return *this;
			} mdl operator+(const pnt& p) const{ mdl r = *this; r += p; return r; }
		    mdl& operator-=(const pnt& p){
				for(size_t i = 0; i < vert_size; i++) vert_list[i] -= p; pivote -= p;
				return *this;
			} mdl operator-(const pnt& p) const{ mdl r = *this; r -= p; return r; }
		    mdl& operator*=(const float _f){
				for(size_t i = 0; i < vert_size; i++) vert_list[i] = (vert_list[i] - pivote) * _f + pivote;
				return *this;
			} mdl operator*(const float _f) const{ mdl r = *this; r *= _f; return r; }
		    mdl& operator/=(const float _f){
				if(_f == 0.0f) return *this;
				for(size_t i = 0; i < vert_size; i++) vert_list[i] = (vert_list[i] - pivote) / _f + pivote;
				return *this;
			} mdl operator/(const float _f) const{ mdl r = *this; r /= _f; return r; }
			// Constructores y destructor
		    mdl() = default; mdl(const mdl& o){
				if(this == &o) return;
				*this = o;
			}
			mdl(decltype(nullptr)) : mdl() {}
			mdl(mdl&& o) noexcept{
				if(this == &o) return;
				*this = o;
			}
			enum class PRIMITIVA : int{
				CUBO     = 0,
				PLANO_XZ = 1,
				PLANO_XY = 2,
				PLANO_YZ = 3,
				PIRAMIDE = 4,
				CONO     = 5,
				ESFERA   = 6,
				CILINDRO = 7,
				LETRA    = -1
			};
			mdl(int mt, float escala = 1.0f, anim aplicar_textura = {}){ // Primitivas
				switch(mt){
					case 0: // Cubo
						agregar_vertice({-1,  1,  1}); agregar_vertice({ 1,  1,  1});
						agregar_vertice({ 1, -1,  1}); agregar_vertice({-1, -1,  1});
						agregar_vertice({ 1,  1, -1}); agregar_vertice({-1,  1, -1});
						agregar_vertice({-1, -1, -1}); agregar_vertice({ 1, -1, -1});
						agregar_cara(0, 1, 2, 3);
						agregar_cara(4, 5, 6, 7);
						agregar_cara(5, 0, 3, 6);
						agregar_cara(1, 4, 7, 2);
						agregar_cara(5, 4, 1, 0);
						agregar_cara(7, 6, 3, 2);
					break;
					case 1: // Plano acostado (XZ)
						agregar_vertice({-1, 0, -1}); agregar_vertice({ 1, 0, -1});
						agregar_vertice({ 1, 0,  1}); agregar_vertice({-1, 0,  1});
						agregar_cara(0, 1, 2, 3);
						agregar_cara(1, 0, 3, 2);
					break;
					case 2: // Plano de pie (XY)
						agregar_vertice({-1,  1, 0}); agregar_vertice({ 1,  1, 0});
						agregar_vertice({ 1, -1, 0}); agregar_vertice({-1, -1, 0});
						agregar_cara(0, 1, 2, 3);
						agregar_cara(1, 0, 3, 2);
					break;
					case 3: // Plano de pie (YZ)
						agregar_vertice({0,  1,  1}); agregar_vertice({0,  1, -1});
						agregar_vertice({0, -1, -1}); agregar_vertice({0, -1,  1});
						agregar_cara(0, 1, 2, 3);
						agregar_cara(1, 0, 3, 2);
					break;
					case 4: // Piramide
						agregar_vertice({-1, -1, -1}); agregar_vertice({ 1, -1, -1});
						agregar_vertice({ 1, -1,  1}); agregar_vertice({-1, -1,  1});
						agregar_vertice({ 0,  1,  0});
						agregar_cara(3, 2, 1, 0);
						agregar_cara(0, 1, 4); agregar_cara(1, 2, 4);
						agregar_cara(2, 3, 4); agregar_cara(3, 0, 4);
					break;
					case 5: // Cono
						agregar_vertice({0, -1, 0}); // centro //  0
						for(int i = 12; i >= 0; i--){
							float a = float(i) * 2 * M_PI / 12;
							agregar_vertice({cos(a), -1, sin(a)});
						}
						agregar_vertice({0, 1, 0});  // punta  // 14
						for(int i = 0; i < 12; i++){
							agregar_cara(0, i + 1, ((i + 1) % 12) + 1);
							agregar_cara(i + 1, 14, ((i + 1) % 12) + 1);
						}
					break;
					case 6: // Esfera
						for(int j = 12; j >= 0; j--){
							float phi = (float(j) / 12) * M_PI;
							for(int i = 12; i >= 0; i--){
								float theta = (float(i) / 12) * 2 * M_PI;
								agregar_vertice({
									sin(phi) * cos(theta),
									cos(phi),
									sin(phi) * sin(theta)
								});
							}
						}
						for(int j = 0; j < 12; j++){
							for(int i = 0; i < 12; i++){
								int p1 = j * 13 + i;
								int p2 = p1 + 13;
								// Corregido para mantener sentido horario desde afuera
								agregar_cara(p1, p2, p2 + 1, p1 + 1);
							}
						}
					break;
					case 7: // Cilindro
						agregar_vertice({0, -1, 0});
						for(int i = 12; i >= 0; i--){
							float a = float(i) * 2 * M_PI / 12;
							agregar_vertice({cos(a), -1, sin(a)}); // base
						}
						agregar_vertice({0,  1, 0});
						for(int i = 0; i <= 12; i++){
							float a = float(i) * 2 * M_PI / 12;
							agregar_vertice({cos(a),  1, sin(a)}); // top
						}
						for(int i = 0; i < 12; i++){
							agregar_cara(14, 14 + i + 1, 14 + ((i + 1) % 12) + 1); // top
							agregar_cara( 0,  0 + i + 1,  0 + ((i + 1) % 12) + 1); // bottom
							// cuerpo
							int a = (12 - i) + 15;
							int b = (12 - (i+1)%12) + 15;
							int c = ((i+1)%12) + 1;
							int d = i + 1;
							agregar_cara(a, b, c, d);
						}
					break;
				}
				(*this) *= escala;
				textura = aplicar_textura;
				rellenar_textura();
			}
			mdl(PRIMITIVA mt, float escala = 1.0f, anim aplicar_textura = {}) : mdl(int(mt), escala, aplicar_textura) {};
			~mdl(){
				limpiar_vertices();
				limpiar_caras();
			}
	};
	
	void mensaje(const DynArray<char> msj, const DynArray<char> title, int method){ MessageBox(nullptr, msj.data(), title.data(), method); }
	bool pregunta(const DynArray<char> msj, const DynArray<char> title, int method, int result){ return MessageBox(nullptr, msj.data(), title.data(), method) == result; }
	int consulta(const DynArray<char> msj, const DynArray<char> title, int method){ return MessageBox(nullptr, msj.data(), title.data(), method); }
	
	void espera(int miliseg){ Sleep(miliseg); }
	
	bool tecla(int key){
		int keyket = stared->__getting_key(key);
		if(keyket == 0) return stared->_teclas_osadas[0];
	    if(stared->_teclas_osadas[keyket]){ stared->_teclas_osadas[keyket] = false; return true; }
	    return false;
	}
	bool tecla_presionada(int key){
		int keyket = stared->__getting_key(key);
		if(keyket == 0) return stared->_teclas_precionadas[0];
		return stared->_teclas_precionadas[keyket];
	}
	
	bool raton(float& x, float& y){
		if(!stared->_raton_dentro) return false;
		x = stared->_xraton; y = stared->_yraton; return true;
	}
	bool raton_dentro(){ return stared->_raton_dentro; }
	float raton_x(){ return stared->_xraton; }
	float raton_y(){ return stared->_yraton; }
	void raton_botones(bool& izq, bool& der){ izq = stared->_bot_izq; der = stared->_bot_der; }
	bool raton_boton_izq(){ return stared->_bot_izq; }
	bool raton_boton_der(){ return stared->_bot_der; }
	bool raton_boton_izq_justo(){
		static bool izqPresionadoAnterior = false;
	    bool izqPresionadoActual = ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0);
	    if(izqPresionadoActual && !izqPresionadoAnterior){
	        izqPresionadoAnterior = true;
	        return true;
	    }
	    izqPresionadoAnterior = izqPresionadoActual;
	    return false;
	}
	bool raton_boton_der_justo(){
		static bool derPresionadoAnterior = false;
	    bool derPresionadoActual = ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0);
	    if (derPresionadoActual && !derPresionadoAnterior) {
	        derPresionadoAnterior = true;
	        return true;
	    }
	    derPresionadoAnterior = derPresionadoActual;
	    return false;
	}
	void raton_capturar(){ SetCapture(stared->hWnd); }
	void raton_liberar(){ ReleaseCapture(); }
	
	int pancho(){ return GetSystemMetrics(SM_CXSCREEN); }
	int palto() { return GetSystemMetrics(SM_CYSCREEN); }
	DynArray<char> vtitulo(){
		char titulo[256];
		int len = GetWindowText(stared->hWnd, titulo, sizeof(titulo));
		if(len > 0) return DynArray<char>(titulo);
		return nullptr;
	}
	void v_fijar_titulo(const DynArray<char> title){ SetWindowText(stared->hWnd, title.data()); }
	int vposicion_x(){ return stared->iXpos; }
	int vposicion_y(){ return stared->iYpos; }
	int vancho(){ return stared->iWidth; }
	int valto(){ return stared->iHeight; }
	void vredimensiona(int ample, int alt){
		stared->iWidth = ample; stared->iHeight = alt;
		SetWindowPos(stared->hWnd, NULL, 0, 0, ample, alt, SWP_NOMOVE | SWP_NOREDRAW);
	}
	void vmover(int x, int y){
		stared->iXpos = x; stared->iYpos = y;
		SetWindowPos(stared->hWnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOREDRAW);
	}
	void vcentrar(){
		vmover((int(GetSystemMetrics(SM_CXSCREEN)) - stared->iWidth) >> 1,
			   (int(GetSystemMetrics(SM_CYSCREEN)) - stared->iHeight) >> 1);
	}
	void vminimiza(){ ShowWindow(stared->hWnd, SW_MINIMIZE); /* minimiza la ventana */ }
	void vmaximiza(){ ShowWindow(stared->hWnd, SW_RESTORE);  /* maximiza la ventana */ }
	void vcompleta(){ ShowWindow(stared->hWnd, SW_MAXIMIZE); /* coloca la ventana de tal forma que ocupe la pantalla por completo */ }
	img vicono(){
		// Primero intenta obtener el ícono usando WM_GETICON
	    HICON hIcon = (HICON)SendMessage(stared->hWnd, WM_GETICON, ICON_SMALL, 0);
	    if(!hIcon) hIcon = (HICON)SendMessage(stared->hWnd, WM_GETICON, ICON_BIG, 0);
	    // Si aún no hay icono, intenta obtenerlo desde la clase de la ventana
	    if(!hIcon){
	        hIcon = (HICON)GetClassLongPtr(stared->hWnd, GCLP_HICONSM);
	        if(!hIcon) hIcon = (HICON)GetClassLongPtr(stared->hWnd, GCLP_HICON);
	    }
	    if(!hIcon) return {};
	    ICONINFO iconInfo;
	    if(!GetIconInfo(hIcon, &iconInfo)) return {};
	    Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromHICON(hIcon);
	    // Limpia recursos de ICONINFO
	    if(iconInfo.hbmMask) DeleteObject(iconInfo.hbmMask);
	    if(iconInfo.hbmColor) DeleteObject(iconInfo.hbmColor);
	    img result(
	    	bitmap->GetWidth(),
	    	bitmap->GetHeight()
		);
		// Define el área a bloquear
	    Gdiplus::Rect rect(0, 0, result.ancho, result.alto);
		Gdiplus::BitmapData data;
		bitmap->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &data);
		if(data.Stride == result.ancho*4) for(size_t j=0,m=result.ancho*result.alto;j<m;j++)result.contenido[j]=((uint32_t*)data.Scan0)[j];
	    bitmap->UnlockBits(&data);
	    delete bitmap;
	    return result;
	}
	void v_fijar_icono(const img& hImage){
		if(!hImage.valido()) return;
	    // Crear un bitmap de color compatible con BGRA
	    BITMAPV5HEADER bi = {};
	    bi.bV5Size        = sizeof(BITMAPV5HEADER);
	    bi.bV5Width       = hImage.ancho;
	    bi.bV5Height      = -(int)hImage.alto; // Negativo: origen superior
	    bi.bV5Planes      = 1;
	    bi.bV5BitCount    = 32;
	    bi.bV5Compression = BI_BITFIELDS;
	    bi.bV5RedMask     = 0x00FF0000;
	    bi.bV5GreenMask   = 0x0000FF00;
	    bi.bV5BlueMask    = 0x000000FF;
	    bi.bV5AlphaMask   = 0xFF000000;
	    void* lpBits = nullptr;
	    HDC hdc = GetDC(nullptr);
	    HBITMAP hBitmap = CreateDIBSection(hdc, (BITMAPINFO*)&bi, DIB_RGB_COLORS, &lpBits, nullptr, 0);
	    ReleaseDC(nullptr, hdc);
	    if (!hBitmap || !lpBits) return;
	    // Convertir ARGB → BGRA
	    uint32_t* dst = (uint32_t*)(lpBits);
	    for(size_t i=0,maxItr=hImage.ancho*hImage.alto;i<maxItr;i++)dst[i]=hImage.contenido[i];
	    // Crear bitmap de máscara (1 bit, sin usarlo realmente)
	    HBITMAP hMonoMask = CreateBitmap(hImage.ancho, hImage.alto, 1, 1, nullptr);
	    ICONINFO iconInfo = {};
	    iconInfo.fIcon    = TRUE;
	    iconInfo.xHotspot = 0;
	    iconInfo.yHotspot = 0;
	    iconInfo.hbmMask  = hMonoMask;
	    iconInfo.hbmColor = hBitmap;
	    HICON hIcon = CreateIconIndirect(&iconInfo);
	    DeleteObject(hMonoMask);
	    DeleteObject(hBitmap);
	    if(!hIcon) return;
	    HWND hwnd = GetActiveWindow(); // Puedes usar tu HWND personalizado si prefieres
	    if(!hwnd){
	        DestroyIcon(hIcon);
	        return;
	    }
	    // Establecer el icono en la ventana
	    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
	    SendMessage(hwnd, WM_SETICON, ICON_BIG,   (LPARAM)hIcon);
	    // Si no se guarda para más tarde, puedes destruirlo cuando ya no se necesite.
	    DestroyIcon(hIcon);
	}
	void voculta(){ ShowWindow(stared->hWnd, SW_HIDE); }
	void vmuestra(){ ShowWindow(stared->hWnd, SW_SHOW); }
	void vcierra(){ PostMessage(stared->hWnd, WM_CLOSE, 0, 0); }
	
	uint32_t color(){ return stared->_color.GetValue(); }
	void fijar_color(const uint32_t& _c){
		stared->_color.SetValue(_c);
		float ex_pen_size = stared->pen->GetWidth();
		if(stared->brush){ delete stared->brush; stared->brush = nullptr; }
		if(stared->pen){ delete stared->pen; stared->pen = nullptr; }
		stared->brush = new Gdiplus::SolidBrush(stared->_color);
		stared->pen = new Gdiplus::Pen(stared->_color, ex_pen_size);
	}
	float lapiz(){ return stared->pen->GetWidth(); }
	void fijar_lapiz(float size){
		if(size < 1.0f) size = 1.0f;
		if(stared->pen){ delete stared->pen; stared->pen = nullptr; }
		stared->pen = new Gdiplus::Pen(stared->_color, size);
	}
	cam camara(){ return stared->_g_cam; }
	void fijar_camara(const cam& _c){ stared->_g_cam = _c; }
	
	void borra(){ stared->grafics->Clear(Gdiplus::Color(255, 0, 0, 0)); stared->zbuffer.set(stared->_g_cam.z_far); }
	void refresca(){
		stared->grafics->Flush();
		InvalidateRect(stared->hWnd, NULL, FALSE);
		InvalidateRect(stared->hWnd, NULL, FALSE); // Invalidar por segunda vez, para garantizar dibujado
	}
	void balde(float x, float y){
		if(!stared->grafics || !stared->backBuffer) return;
		Gdiplus::Color targetColor;
		stared->backBuffer->GetPixel((int)x, (int)y, &targetColor);
		uint32_t targetValue = targetColor.GetValue();
		uint32_t fillValue = stared->_color.GetValue();
		if(fillValue == targetValue) return;
		struct __balde__{
			static void exe(const int& x, const int& y, const uint32_t& targetValue){
				if(x < 0 || y < 0 || x >= stared->iWidth || y >= stared->iHeight) return;
				Gdiplus::Color currentColor;
				stared->backBuffer->GetPixel(x, y, &currentColor);
				if(currentColor.GetValue() != targetValue) return;
				stared->grafics->FillRectangle(stared->brush, (float)x, (float)y, 1.0f, 1.0f);
				exe(x + 1, y, targetValue);
				exe(x - 1, y, targetValue);
				exe(x, y + 1, targetValue);
				exe(x, y - 1, targetValue);
			}
		}; __balde__::exe((int)x, (int)y, targetValue);
	}
	void punto(float x, float y){ stared->grafics->FillRectangle(stared->brush, x, y, 1.0f, 1.0f); }
	void linea(float x1, float y1, float x2, float y2){ stared->grafics->DrawLine(stared->pen, x1, y1, x2, y2); }
	void rectangulo(float x1, float y1, float x2, float y2)
	{stared->grafics->DrawRectangle(stared->pen,(x1<x2?x1:x2),(y1<y2?y1:y2),(x2<x1?(x1-x2):(x2-x1)),(y2<y1?(y1-y2):(y2-y1)));}
	void rectangulo_lleno(float x1, float y1, float x2, float y2)
	{stared->grafics->FillRectangle(stared->brush,(x1<x2?x1:x2),(y1<y2?y1:y2),(x2<x1?(x1-x2):(x2-x1)),(y2<y1?(y1-y2):(y2-y1)));}
	void circulo(float x_cen, float y_cen, float radio)
	{ stared->grafics->DrawEllipse(stared->pen, x_cen - radio, y_cen - radio, 2 * radio, 2 * radio); }
	void circulo_lleno(float x_cen, float y_cen, float radio)
	{ stared->grafics->FillEllipse(stared->brush, x_cen - radio, y_cen - radio, 2 * radio, 2 * radio); }
	void elipse(float x_cen, float y_cen, float radio, float anguloInicio, float anguloFinal){
		anguloInicio = fmod__x(anguloInicio, 360.0f);
		anguloFinal  = fmod__x(anguloFinal,  360.0f);
		stared->grafics->DrawArc(stared->pen,
			x_cen - radio, y_cen - radio,
			 2.0f * radio,  2.0f * radio,
			-(anguloInicio<anguloFinal?anguloInicio:anguloFinal),
			-(anguloInicio<anguloFinal?(anguloFinal-anguloInicio):(anguloInicio-anguloFinal))
		);
	}
	void elipse_lleno(float x_cen, float y_cen, float radio, float anguloInicio, float anguloFinal){
		anguloInicio = fmod__x(anguloInicio, 360.0f);
		anguloFinal  = fmod__x(anguloFinal,  360.0f);
		stared->grafics->FillPie(stared->brush,
			x_cen - radio, y_cen - radio,
			 2.0f * radio,  2.0f * radio,
			-(anguloInicio<anguloFinal?anguloInicio:anguloFinal),
			-(anguloInicio<anguloFinal?(anguloFinal-anguloInicio):(anguloInicio-anguloFinal))
		);
	}
	
	void arco(float x1, float y1, float x2, float y2, float x3, float y3){
		// Vectores intermedios
	    float A1 = x2 - x1, B1 = y2 - y1;
	    float A2 = x3 - x2, B2 = y3 - y2;
	    float C1 = (A1 * (x1 + x2) + B1 * (y1 + y2)) / 2.0f;
	    float C2 = (A2 * (x2 + x3) + B2 * (y2 + y3)) / 2.0f;
	    float D = A1 * B2 - A2 * B1;
	    if((D<0.0f?(-D):D) < 1e-5) return; // puntos colineales
	    // Centro del círculo
	    float cx = (C1 * B2 - C2 * B1) / D;
	    float cy = (A1 * C2 - A2 * C1) / D;
	    // Radio
	    float dx = x1 - cx, dy = y1 - cy;
	    float radio = square_root(dx*dx + dy*dy);
	    // Ángulos
	    float a1 = M_RAD2DEG(fast_atan2(y1 - cy, x1 - cx));
	    float a2 = M_RAD2DEG(fast_atan2(y2 - cy, x2 - cx));
	    float a3 = M_RAD2DEG(fast_atan2(y3 - cy, x3 - cx));
	    // Normalizamos
	    while (a2 < a1) a2 += 360;
	    while (a3 < a1) a3 += 360;
	    float angInicio = a1;
	    float angFin = a3;
	    float angBarrido = angFin - angInicio;
	    if(!(a1 < a2 && a2 < a3)) angBarrido -= 360; // Verificamos dirección: si punto medio no está entre inicio y fin, invertir barrido
	    stared->grafics->DrawArc(stared->pen, cx - radio, cy - radio, 2 * radio, 2 * radio, angInicio, angBarrido); // Dibujar
	}
	void arco_lleno(float x1, float y1, float x2, float y2, float x3, float y3){
		// Vectores intermedios
	    float A1 = x2 - x1, B1 = y2 - y1;
	    float A2 = x3 - x2, B2 = y3 - y2;
	    float C1 = (A1 * (x1 + x2) + B1 * (y1 + y2)) / 2.0f;
	    float C2 = (A2 * (x2 + x3) + B2 * (y2 + y3)) / 2.0f;
	    float D = A1 * B2 - A2 * B1;
	    if((D<0.0f?(-D):D) < 1e-5) return; // puntos colineales
	    // Centro del círculo
	    float cx = (C1 * B2 - C2 * B1) / D;
	    float cy = (A1 * C2 - A2 * C1) / D;
	    // Radio
	    float dx = x1 - cx, dy = y1 - cy;
	    float radio = square_root(dx*dx + dy*dy);
	    // Ángulos
	    float a1 = M_RAD2DEG(fast_atan2(y1 - cy, x1 - cx));
	    float a2 = M_RAD2DEG(fast_atan2(y2 - cy, x2 - cx));
	    float a3 = M_RAD2DEG(fast_atan2(y3 - cy, x3 - cx));
	    // Normalizamos
	    while (a2 < a1) a2 += 360;
	    while (a3 < a1) a3 += 360;
	    float angInicio = a1;
	    float angFin = a3;
	    float angBarrido = angFin - angInicio;
	    if(!(a1 < a2 && a2 < a3)) angBarrido -= 360; // Verificamos dirección: si punto medio no está entre inicio y fin, invertir barrido
		stared->grafics->FillPie(stared->brush, cx - radio, cy - radio, 2 * radio, 2 * radio, angInicio, angBarrido); // Dibujar
	}
	void triangulo(float x1, float y1, float x2, float y2, float x3, float y3){
		Gdiplus::PointF puntos[3] = {
	        Gdiplus::PointF(x1, y1),
	        Gdiplus::PointF(x2, y2),
	        Gdiplus::PointF(x3, y3)
	    };
	    stared->grafics->DrawPolygon(stared->pen, puntos, 3);
	}
	void triangulo_lleno(float x1, float y1, float x2, float y2, float x3, float y3){
		Gdiplus::PointF puntos[3] = {
	        Gdiplus::PointF(x1, y1),
	        Gdiplus::PointF(x2, y2),
	        Gdiplus::PointF(x3, y3)
	    };
	    stared->grafics->FillPolygon(stared->brush, puntos, 3);
	}
	void poligono(DynArray<float> _x, DynArray<float> _y){
		if(!_x.data() || !_y.data()) return;
		if(_x.size() != _y.size() || _x.size() < 2) return;
		Gdiplus::PointF puntos[_x.size()];
		for(size_t i = 0; i < _x.size(); i++) puntos[i] = Gdiplus::PointF(_x[i], _y[i]);
	    stared->grafics->DrawPolygon(stared->pen, puntos, _x.size());
	}
	void poligono_lleno(DynArray<float> _x, DynArray<float> _y){
		if(!_x.data() || !_y.data()) return;
		if(_x.size() != _y.size() || _x.size() < 2) return;
		Gdiplus::PointF puntos[_x.size()];
		for(size_t i = 0; i < _x.size(); i++) puntos[i] = Gdiplus::PointF(_x[i], _y[i]);
	    stared->grafics->FillPolygon(stared->brush, puntos, _x.size());
	}
	void texto(float x, float y, const DynArray<char>& txt){
		if(!stared) return;
		if(!stared->grafics || !stared->brush || txt.empty()) return;
		DynArray<wchar_t> wtxt(txt.begin(), txt.end()),
					      wnombre(stared->g_fuente_actual.nombre.begin(), stared->g_fuente_actual.nombre.end());
	    Gdiplus::FontStyle estilo = Gdiplus::FontStyleRegular;
	    if(stared->g_fuente_actual.negrita)    estilo = (Gdiplus::FontStyle)(estilo | Gdiplus::FontStyleBold);
	    if(stared->g_fuente_actual.italica)    estilo = (Gdiplus::FontStyle)(estilo | Gdiplus::FontStyleItalic);
	    if(stared->g_fuente_actual.subrayada)  estilo = (Gdiplus::FontStyle)(estilo | Gdiplus::FontStyleUnderline);
	    Gdiplus::Font* fuente = nullptr;
	    size_t index = stared->g_fuentes_defecto;
	    for(size_t i = 0; i < stared->g_fuentes_defecto; i++) if(wnombre == stared->g_nombres_familia[i]){
	    	index = i;
	    	break;
		}
	    if(index < stared->g_fuentes_defecto) fuente = new Gdiplus::Font(wnombre.data(), stared->g_fuente_actual.dimension, estilo, Gdiplus::UnitPixel); // Fuente del sistema
	    else{
	        // Fuente personalizada
	        Gdiplus::FontFamily familias[64];
	        int cantidad = 0; stared->g_fuentes.GetFamilies(64, familias, &cantidad);
	        if(index - stared->g_fuentes_defecto < cantidad) fuente = new Gdiplus::Font(
				&familias[index - stared->g_fuentes_defecto],
				stared->g_fuente_actual.dimension,
				estilo,
				Gdiplus::UnitPixel
			);
	    }
	    if(fuente && fuente->IsAvailable()){
		    Gdiplus::GraphicsState state = stared->grafics->Save(); // Guardar el estado original del gráfico
		    // Trasladar al punto deseado y rotar
		    stared->grafics->TranslateTransform(x, y);
		    stared->grafics->RotateTransform(stared->g_fuente_actual.angulo);
		    stared->grafics->DrawString(wtxt.data(), -1, fuente, Gdiplus::PointF(0, 0), stared->brush);
		    stared->grafics->Restore(state); // Restaurar
		}
	    delete fuente;
	}
	void imagen(float x, float y, const DynArray<char>& filePath){ imagen(x, y, cargar_imagen(filePath)); }
	void imagen(float x, float y, const img& hImage){
	    if(!hImage.contenido || hImage.ancho <= 0 || hImage.alto <= 0) return;
	    Gdiplus::Bitmap bitmap(hImage.ancho, hImage.alto, hImage.ancho * 4, PixelFormat32bppARGB, (BYTE*)hImage.contenido);
	    stared->grafics->DrawImage(&bitmap, x, y);
	}
	void imagen(float x, float y, anim& hAnim){
		imagen(x, y, hAnim.obtener_frame());
		hAnim.siguiente_frame();
	}
	void imagen(const pnt& hPoint){
		float size = (stared->iWidth>stared->iHeight?stared->iWidth:stared->iHeight) / 200.0f;
		size = size > 1.0f ? size : 1.0f;
		circulo_lleno(hPoint.x, hPoint.y, size);
	}
	void imagen(float x, float y, const mdl& hModel){
	    if(!stared->backBuffer) return;
		const float foco = stared->_g_cam.focal();
	    pnt luz_dir(1, 0, 1);
	    luz_dir = -luz_dir.normalizar();
	    const float luz_ambiente = 0.75f;
		const float epsilon = 1e-5f;
	    pnt cards[hModel.cantidad_vertices()];
		for(size_t i = 0, j = 0; i < hModel.cantidad_caras(); i++){
			pnt my_normal(hModel.normal_de(i));
			for(size_t j = 0, plop = hModel.obtener_cara(i, 0); j < hModel.longitud_cara(i); plop = hModel.obtener_cara(i, j++))
				cards[plop] = (cards[plop] + my_normal).normalizar();
		}
		size_t directions[hModel.cantidad_caras()]{0};
		for(size_t i = 0; i < hModel.cantidad_caras(); i++)
			directions[i] = (i == 0) ? 0 : directions[i - 1] + hModel.datos_longitudes_faces()[i - 1];
		// Bloquear bitmap
		Gdiplus::BitmapData data;
		Gdiplus::Rect rect(0, 0, stared->iWidth, stared->iHeight);
		stared->backBuffer->LockBits(&rect, Gdiplus::ImageLockModeWrite | Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &data);
		BYTE* base = static_cast<BYTE*>(data.Scan0);
	    struct InterseccionUV{
		    float x, z;
		    pnt uv, normal;
		};
		for(size_t i = 0; i < hModel.cantidad_caras(); i++){
		    if(hModel.longitud_cara(i) < 3) continue;
		    DynArray<pnt> _vrs, _nrm, _uvs;
		    bool fuera_izq = true, fuera_der = true, fuera_arr = true, fuera_abj = true;
		    for(int j = 0; j < hModel.longitud_cara(i); j++){
		        size_t idx = hModel.datos_faces()[directions[i] + j];
		        if(idx >= hModel.cantidad_vertices()) continue;
		        pnt punto(hModel.datos_vertices()[idx] - hModel.pivote - stared->_g_cam.posicion);
		        if(punto.z >= 0) continue;
		        _vrs.emplace_back(
					foco * punto.x / punto.z + stared->iWidth  * 0.5f,
		            foco * punto.y / punto.z + stared->iHeight * 0.5f,
			        -punto.z
				);
				if(_vrs.back().x >= 0)      fuera_izq = false;
				if(_vrs.back().x < stared->iWidth)  fuera_der = false;
				if(_vrs.back().y >= 0)      fuera_arr = false;
				if(_vrs.back().y < stared->iHeight) fuera_abj = false;
				_nrm.push_back(cards[idx]);
				pnt aux; descompress_dot(hModel.datos_uvs()[directions[i] + j], &aux);
				_uvs.push_back(aux);
		    }
		    if(fuera_izq || fuera_der || fuera_arr || fuera_abj || _vrs.size() < 3) continue;
		    pnt normal = hModel.normal_de(i);
		    pnt centro = { 0, 0, 0 };
		    for(int j = 0; j < hModel.longitud_cara(i); j++) centro += hModel.obtener_vertice(hModel.obtener_cara(i, j));
		    centro = (centro / (float)hModel.longitud_cara(i)) - hModel.pivote;
		    pnt vista = stared->_g_cam.posicion - centro;
		    if((normal * vista) >= -epsilon) continue;
		    uint32_t color_final = stared->_color.GetValue();
		    if(hModel.brillo){
		    	float luz_total = normal * luz_dir;
				luz_total = luz_ambiente + (luz_total > 0.0f ? luz_total : 0.0f) * luz_ambiente;
		    	luz_total = luz_total < 1.0f ? luz_total : 1.0f;
		        BYTE r = stared->_color.GetRed()   * luz_total;
		        BYTE g = stared->_color.GetGreen() * luz_total;
		        BYTE b = stared->_color.GetBlue()  * luz_total;
		        BYTE a = stared->_color.GetAlpha();
		        stared->_color.SetValue(RGBA(r, g, b, a));
		    }
		    /*{ // Render
		    	bool _text_valid = false;
				if(hModel.longitud_cara(i) == 3)
					_text_valid = hModel.textura_valida(_uvs[0].z) && hModel.textura_valida(_uvs[1].z) && hModel.textura_valida(_uvs[2].z) &&
						_uvs[0].x*(_uvs[1].y - _uvs[2].y) + _uvs[1].x*(_uvs[2].y - _uvs[0].y) + _uvs[2].x*(_uvs[0].y - _uvs[1].y) > 0;
		    	else // Solo caera 4, dado que 4 es la cantidad maxima, y ya hemos descartado al 2 (cantidad minima) con antelacion
					_text_valid = hModel.textura_valida(_uvs[0].z) && hModel.textura_valida(_uvs[1].z) &&
								  hModel.textura_valida(_uvs[2].z) && hModel.textura_valida(_uvs[3].z) &&
						(_uvs[0].x*(_uvs[1].y - _uvs[2].y) + _uvs[1].x*(_uvs[2].y - _uvs[0].y) + _uvs[2].x*(_uvs[0].y - _uvs[1].y) > 0.0f ||
						 _uvs[0].x*(_uvs[3].y - _uvs[2].y) + _uvs[3].x*(_uvs[2].y - _uvs[0].y) + _uvs[2].x*(_uvs[0].y - _uvs[3].y) > 0.0f);
				int vy[_vrs.size()]; for(size_t i = 0; i < _vrs.size(); i++) vy[i] = _vrs[i].y;
				int minY = vy[0], maxY = vy[0]; for(int* first = vy; first < vy + _vrs.size(); first++){
					if(*first < minY) minY = *first;
					if(*first > maxY) maxY = *first;
				}
			    minY = minY >          0 ? minY :          0;
			    maxY = maxY < iHeight -1 ? maxY : iHeight -1;
			    // Post color
			    uint32_t post_color = _color.GetValue();
			    for(int y = minY; y <= maxY; y++){
			    	DynArray<InterseccionUV> intersecciones;
				    for(size_t i = 0; i < _vrs.size(); i++){
					    size_t j = (i + 1) % _vrs.size();
					    if((vy[i] <= y && vy[j] > y) || (vy[j] <= y && vy[i] > y)){
					        float t = float(y - vy[i]) / float(vy[j] - vy[i]);
					        intersecciones.push_back({
					        	_vrs[i].x + t * (_vrs[j].x - _vrs[i].x),
								_vrs[i].z + t * (_vrs[j].z - _vrs[i].z),
								_uvs[i]   + t * (_uvs[j]   - _uvs[i]),
								_nrm[i]   + t * (_nrm[j]   - _nrm[i])
							});
					    }
					}
				    fast_sort(intersecciones.begin(), intersecciones.end(), [](const InterseccionUV& a, const InterseccionUV& b){ return a.x < b.x; });
				    for(size_t i = 0; i + 1 < intersecciones.size(); i += 2){
					    const InterseccionUV& izq = intersecciones[i];
					    const InterseccionUV& der = intersecciones[i + 1];
					    float x1 = izq.x >      0.0f ? izq.x :      0.0f;
					    float x2 = der.x < iWidth -1 ? der.x : iWidth -1;
					    float z1 = izq.z;
					    float z2 = der.z;
					    for(int x = x1; x <= int(x2); x++) if(x >= 0 && x < iWidth && y >= 0 && y < iHeight){
					        float t = (x2 == x1) ? 0.0f : float(x - x1) / float(x2 - x1);
					        float z_interp = z1 + t * (z2 - z1);
					        float* zpixl = &zbuffer[y * iWidth + x];
					        if(z_interp < *zpixl){
					            BYTE* pixel = base + y * data.Stride + x * 4;
					            uint32_t _merge = 0x00000000U,
					                	 _src_color = post_color;
					            if(hModel.suavizado){
				                    pnt normal_interp = izq.normal + t * (der.normal - izq.normal);
					                normal_interp = normal_interp.normalizar();
					                float luz = clamp<float>(normal_interp * luz_dir, 0.0f, 1.0f);
					                _src_color = multiplicar_colores(RGBA(luz * 0xFF, luz * 0xFF, luz * 0xFF, 0xFF), _src_color);
				                }
					            if(_text_valid){ // Interpolación de UV entre izq y der
					                pnt uv_interp = (izq.uv + ((der.x == izq.x) ? 0.0f : float(x - izq.x) / float(der.x - izq.x)) * (der.uv - izq.uv));
					                uv_interp.x = (hModel.textura.ancho == 1) ? 0 : clamp<int>(uv_interp.x, 0, hModel.textura.ancho - 1);
					                uv_interp.y = (hModel.textura.alto  == 1) ? 0 : clamp<int>(uv_interp.y, 0, hModel.textura.alto  - 1);
					                printf("lelele.\n");
					                _merge = multiplicar_colores(mezclar_colores((pixel[3] << 24) | (pixel[2] << 16) | (pixel[1] << 8) | pixel[0], hModel.textura.at(
										clamp<size_t>(uv_interp.z, 0, hModel.textura.cantidad - 1),
										uv_interp.x,
						                uv_interp.y
									)), _src_color);
					            }else _merge = mezclar_colores((pixel[3] << 24) | (pixel[2] << 16) | (pixel[1] << 8) | pixel[0], _src_color);
					            pixel[0] = GetBlue(_merge);
					            pixel[1] = GetGreen(_merge);
					            pixel[2] = GetRed(_merge);
					            pixel[3] = GetAlpha(_merge);
					            *zpixl = z_interp;
					        }
					    }
					}
				}
			}*/
		    stared->_color.SetValue(color_final);
		}
	    stared->backBuffer->UnlockBits(&data);
	}
	void imagen(const mdl& hModel){ imagen(0, 0, hModel); }
	
	fnt fuente(){ return stared->g_fuente_actual; }
	void fijar_fuente(const fnt& _fft){ stared->g_fuente_actual = _fft; }
	void fijar_fuente(const DynArray<char>& fuente, unsigned int dimension, unsigned int _styles){
		if(fuente.empty()) return;
		fnt auxiliary_font;
		auxiliary_font.nombre    = fuente;
		auxiliary_font.dimension = dimension,
		auxiliary_font.italica   = (_styles & 0x4) > 0;
		auxiliary_font.subrayada = (_styles & 0x2) > 0;
		auxiliary_font.negrita   = (_styles & 0x1) > 0;
		fijar_fuente(auxiliary_font);
	}
	DynArray<DynArray<char>> obtener_fuentes(){
		DynArray<DynArray<char>> result;
		for(size_t i = 0; i < stared->g_nombres_familia.size(); i++)
			result.emplace_back(stared->g_nombres_familia[i].begin(), stared->g_nombres_familia[i].end());
		return result;
	}
	
	img cargar_imagen(const DynArray<char>& filePath){
		printf("%s|%zu\n", filePath.data(), filePath.size());
	    if(filePath.empty()) return {};
	    FILE* file = fopen(filePath.data(), "rb");  // Abrir en modo binario
    	if(!file) return {}; // Retornar vacio si falla la apertura
    	char header[8];
	    size_t leidos = fread(header, 1, 8, file);
	    fclose(file);
	    if(header[0] == 'B' && header[1] == 'M'){ // BitMap
	    	Gdiplus::Bitmap* hImage = Gdiplus::Bitmap::FromFile(DynArray<wchar_t>(filePath.begin(), filePath.end()).data(), FALSE);
		    if(!hImage || hImage->GetLastStatus() != Gdiplus::Ok){
		    	mensaje("No se pudo cargar la imagen", "Error", MB_OK | MB_ICONERROR);
		        return {}; // Devuelve imagen vacía
		    }
		    img result(hImage->GetWidth(), hImage->GetHeight());
			Gdiplus::Rect rect(0, 0, result.ancho, result.alto);
			Gdiplus::BitmapData data;
			hImage->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &data);
			if(data.Stride == result.ancho * 4) for(size_t j=0,m=result.ancho*result.alto;j<m;j++)result.contenido[j]=((uint32_t*)data.Scan0)[j];
			hImage->UnlockBits(&data);
			delete hImage;
		    return result;
		}
		if(!((header[0] == char(0x89) && header[1] == 'P' && header[2] == 'N' && header[3] == 'G') || (header[0] == 0xFF && header[1] == 0xD8)
			|| ((header[0] == 'I' && header[1] == 'I') || (header[0] == 'M' && header[1] == 'M')))) return {};
	    Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromFile(DynArray<wchar_t>(filePath.begin(), filePath.end()).data(), false);
	    if(!bitmap || bitmap->GetLastStatus() != Gdiplus::Ok){
	        mensaje("No se pudo cargar la imagen con GDI+", "Error", MB_OK | MB_ICONERROR);
	        delete bitmap;
	        return {};
	    }
	    size_t w = bitmap->GetWidth(),
	           h = bitmap->GetHeight();
	    img result(w, h);
	    for(size_t y = 0; y < h; y++) for(size_t x = 0; x < w; x++){
	        Gdiplus::Color color;
	        if(bitmap->GetPixel(x, y, &color) == Gdiplus::Ok) result.at(x, y) = RGBA(color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha());
	    }
	    delete bitmap;
	    return result;
	}
	anim cargar_animacion(const DynArray<char>& gifPath){
		Gdiplus::Bitmap gif(DynArray<wchar_t>(gifPath.begin(), gifPath.end()).data());
	    if(gif.GetLastStatus() != Gdiplus::Ok) return {};
	    UINT count = gif.GetFrameCount(&Gdiplus::FrameDimensionTime);
	    if(count == 0) return {};
	    // Obtener tiempos
	    UINT size = gif.GetPropertyItemSize(PropertyTagFrameDelay);
	    DynArray<BYTE> delayData;
	    Gdiplus::PropertyItem* delayItem = nullptr;
	    DynArray<int> delays;
	    if(size > 0){
	        delayData.resize(size);
	        if(gif.GetPropertyItem(PropertyTagFrameDelay, size, reinterpret_cast<Gdiplus::PropertyItem*>(delayData.data())) == Gdiplus::Ok){
	            delayItem = (Gdiplus::PropertyItem*)(delayData.data());
	            size_t numDelays = delayItem->length;
	            if(delayItem->type == PropertyTagTypeLong)  numDelays /=   sizeof(int);
	            if(delayItem->type == PropertyTagTypeShort) numDelays /= sizeof(short);
	            delays.emplace((int*)(delayItem->value), (int*)(delayItem->value) + numDelays);
	        }
	    }
	    anim result(gif.GetWidth(), gif.GetHeight());
	    for(UINT i = 0; i < count; ++i){
	        gif.SelectActiveFrame(&Gdiplus::FrameDimensionTime, i);
	        Gdiplus::Bitmap frame(gif.GetWidth(), gif.GetHeight(), PixelFormat32bppARGB);
	        Gdiplus::Graphics g(&frame);
	        g.DrawImage(&gif, 0, 0);
	        Gdiplus::BitmapData data;
	        Gdiplus::Rect rect(0, 0, gif.GetWidth(), gif.GetHeight());
	        img cuadro(gif.GetWidth(), gif.GetHeight());
	        if(frame.LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &data) == Gdiplus::Ok){
	            if(data.Stride == cuadro.ancho * 4) for(size_t j=0,m=cuadro.ancho*cuadro.alto;j<m;j++)cuadro.contenido[j]=((uint32_t*)data.Scan0)[j];
	            frame.UnlockBits(&data);
	        }
	        unsigned int tiempo = 1; // valor por defecto si no hay delays
	        if(i < delays.size()) tiempo = float(delays[i] > 1 ? delays[i] : 1) / 10.0f;
	        result.agregar_frame(cuadro, tiempo);
	    }
	    return result;
	}
	int ancho_imagen(const DynArray<char>& filePath){ return cargar_imagen(filePath).ancho; }
	int alto_imagen(const DynArray<char>& filePath){ return cargar_imagen(filePath).alto; }
	fnt cargar_fuente(const DynArray<char>& filePath){
		if(filePath.empty()) return {};
		size_t wlen = mbstowcs(nullptr, filePath.data(), filePath.size());
		if(wlen == (size_t)-1) return {};
		wchar_t wPath[wlen]; mbstowcs(wPath, filePath.data(), wlen);
	    // Agregar a la colección
	    if(stared->g_fuentes.AddFontFile(wPath) != Gdiplus::Ok) return {}; // Error al cargar la fuente
	    // Obtener el nombre de la familia añadida
	    Gdiplus::FontFamily familias[64];
	    int cantidad = 0;
	    stared->g_fuentes.GetFamilies(64, familias, &cantidad);
	    for(int i = 0; i < cantidad; i++){
	        wchar_t wname[LF_FACESIZE]{0};
	        familias[i].GetFamilyName(wname);
	        if(__find(stared->g_nombres_familia.begin(), stared->g_nombres_familia.end(), wname) == stared->g_nombres_familia.end()){
	        	stared->g_nombres_familia.emplace_back(wname, wname + LF_FACESIZE);
				size_t len = wcstombs(nullptr, wname, 0);
				if(len == 0 || len == (size_t)-1) return {};
				char sname[len]; wcstombs(sname, wname, len);
	            return fnt(DynArray<char>(&sname[0], sname + len));
	        }
	    }
	    return fnt(stared->g_nombres_familia.back());
	}
	mdl cargar_modelo(const DynArray<char>& filePath, double scale, bool load_textures){
		// Obtener extensión
	    char* base_path = nullptr;
	    size_t base_path_size = 0;
		{
			const char* ext = filePath.end() - 1; while(ext && *ext != '.' && ext != filePath.begin()) ext--;
			ext++; base_path_size = ext - filePath.begin();
			base_path = new char[base_path_size];
		    ext--; for(int i = 0; (++ext) && ext != filePath.end(); i++){
				base_path[i] = (*ext >= 'A' && *ext <= 'Z'
					? (*ext - 'A' + 'a')
					: *ext
				);
			}
		}
	    if(base_path_size < 3 || base_path[0]!='o' || base_path[1]!='b' || base_path[2]!='j'){
	    	delete[] base_path;
			return {};
		}
	    delete[] base_path; base_path = nullptr; base_path_size = 0;
	    // Cargar modelo
		FILE* file = fopen(filePath.data(), "r");
    	if(!file) return {};
	    {
	        char* last_slash = (char*)(&(filePath.end()[0])) - 1;
			while(last_slash && *last_slash != '/' && last_slash != (char*)(&(filePath.begin()[0]))) last_slash--;
			if(!last_slash || last_slash == (char*)(&(filePath.begin()[0]))){
				last_slash = (char*)(&(filePath.end()[0])) - 1;
				while(last_slash && *last_slash != '\\' && last_slash != (char*)(&(filePath.begin()[0]))) last_slash--;
			}
			if(last_slash){
				base_path_size = last_slash - filePath.begin() + 1;
				base_path = new char[base_path_size + 1];
				for(size_t j=0;j<base_path_size;j++)base_path[j]=filePath[j];
	            base_path[base_path_size] = '\0';
			}
	    }
	    if(!base_path) return {};
	    mdl result;
	    DynArray<pnt> temp_vertices, temp_uvs;
	    DynArray<DynArray<char>> textures_names;
	    char* linea = nullptr;
	    size_t capacidad = 0, tamano = 0;
		for(size_t numLinea = 0, uv_idx = 0; ([&]() -> int{
			if(!file) return EOF;
			if(linea) delete[] linea; linea = new char[1]{'\0'};
			tamano = 0; capacidad = 0;
		    int c; while((c = fgetc(file)) != EOF){
		        if(tamano + 1 >= capacidad){
		            size_t nuevaCap = capacidad == 0 ? 16 : (capacidad << 1);
		            char* nuevo = new char[nuevaCap];
		            if(linea){
		            	for(size_t j=0;j<tamano;j++)nuevo[j]=linea[j];
		                delete[] linea;
		            }
		            linea = nuevo;
		            capacidad = nuevaCap;
		        }
		        linea[tamano++] = ((c == '\n') ? '\0' : (char)c);
		        if(c == '\n') break;
		    }
		    linea[tamano] = '\0';
		    return c;
		}()) != EOF; numLinea++){
	        if(tamano >= 3) if(linea[0] == 'v'){
	        	double x, y, z;
				sscanf(linea + 2, "%f %f %f", &x, &y, &z);
				x *= scale;
				y *= scale;
				z *= scale;
            	temp_vertices.push_back({(float)x, (float)y, (float)z});
			}
	        if(tamano >= 3) if(linea[0] == 'f'){
	            DynArray<size_t> vert_indices;
	            DynArray<pnt> uv_coords;
	            for(char* token = strtok(linea + 2, " "); token; token = strtok(nullptr, " ")){
	                int vi = 0, ti = 0;
	                char* p1 = strchr(token, '/');
	                if(p1){
	                    *p1 = '\0';
	                    vi = atoi(token);
	                    char* p2 = strchr(p1 + 1, '/');
	                    if(p2){
	                        *p2 = '\0';
	                        ti = atoi(p1 + 1);
	                    }else ti = atoi(p1 + 1);
	                }else vi = atoi(token);
	                vert_indices.push_back(vi > 0 ? (vi - 1) : (temp_vertices.size() + vi));
	                size_t idx_t = ti > 0 ? (ti - 1) : 0;
	                uv_coords.push_back((ti > 0 && idx_t < temp_uvs.size()) ? temp_uvs[idx_t] : pnt{0.0f, 0.0f, (float)uv_idx});
	            }
				if(vert_indices.size() == 3) result.agregar_cara(
					vert_indices[2], vert_indices[1], vert_indices[0], // Inversion de vertices y UVs
					uv_coords[2], uv_coords[1], uv_coords[0]		   // Debido a que maxiwin los maneja en orden inverso ...
				);
				else if(vert_indices.size() == 4) result.agregar_cara(
					vert_indices[3], vert_indices[2], vert_indices[1], vert_indices[0], // ... Es decir,
					uv_coords[3], uv_coords[2], uv_coords[1], uv_coords[0] // Las caras visibles pasan de ser las de sentido antihorario a sentido horario ...
				);
				else if(vert_indices.size() > 4) for(size_t i = 1; i + 1 < vert_indices.size(); ++i) result.agregar_cara( // Triangulación simple en abanico para n-gonos
					vert_indices[i + 1], vert_indices[i], vert_indices[0], // ... Y diseversa.
					uv_coords[i + 1], uv_coords[i], uv_coords[0]
				);
			}
			if(load_textures){
				if(tamano >= 2) if(linea[0] == 'v' && linea[1] == 't'){
		            float u, v;
		            sscanf(linea + 3, "%f %f", &u, &v);   ///////////
		            temp_uvs.push_back({u * result.textura.ancho, v * result.textura.alto, (float)uv_idx});
				}
				if(tamano >= 8) if(linea[0] == 'm' && linea[1] == 't' && linea[2] == 'l' && linea[3] == 'l' && linea[4] == 'i' && linea[5] == 'b'){
					// Load a .mtl
					char mtl_name[base_path_size + (tamano - 8) + 1]{'\0'}; {
						for(size_t i=0;i<base_path_size;i++)mtl_name[i]=base_path[i];
			        	char format[16]{'\0'};
						snprintf(format, sizeof(format), "%%%zus", tamano - 8);
						sscanf(linea + 7, format, mtl_name + base_path_size);
			        }
					FILE* mtl = fopen(mtl_name, "r");
		            if(!mtl) continue;
				    for(size_t mtl_numLinea = 0; ([&]() -> int{
						if(!mtl) return EOF;
						if(linea) delete[] linea; linea = new char[1]{'\0'};
						tamano = 0; capacidad = 0;
					    int c; while((c = fgetc(mtl)) != EOF){
					        if(tamano + 1 >= capacidad){
					            size_t nuevaCap = capacidad == 0 ? 16 : (capacidad << 1);
					            char* nuevo = new char[nuevaCap];
					            if(linea){
					                for(size_t j=0;j<tamano;j++)nuevo[j]=linea[j];
					                delete[] linea;
					            }
					            linea = nuevo;
					            capacidad = nuevaCap;
					        }
					        linea[tamano++] = ((c == '\n') ? '\0' : (char)c);
					        if(c == '\n') break;
					    }
					    linea[tamano] = '\0';
					    return c;
					}()) != EOF; mtl_numLinea++){
				    	if(tamano >= 8) if(linea[0] == 'n' && linea[1] == 'e' && linea[2] == 'w' && linea[3] == 'm' && linea[4] == 't' && linea[5] == 'l'){
				    		char tex_path[tamano - 7]{'\0'};
							sscanf(linea + 7, "%s", tex_path);
					        textures_names.push_back(tex_path);
				    		result.textura.agregar_frame(img(1U,1U));
						}
		                if(tamano >= 4) if(linea[0] == 'm' && linea[1] == 'a' && linea[2] == 'p' && linea[3] == '_'){
		                	if(tamano >= 8) if(linea[4] == 'K' && linea[5] == 'd'){
								DynArray<char> post_linea(linea); post_linea.erase(0, 7);
		                		DynArray<char> tex_path = base_path; tex_path += post_linea;
			                    img aux_img(cargar_imagen(tex_path));
			                    if(aux_img.valido()){
			                    	result.textura.eliminar_frame(result.textura.cantidad-1);
			                    	if(result.textura.valido()){
			                    		unsigned int maxWidth  = result.textura.ancho > aux_img.ancho ? result.textura.ancho : aux_img.ancho,
										             maxHeight = result.textura.alto  > aux_img.alto  ? result.textura.alto  : aux_img.alto;
										if(result.textura.ancho < aux_img.ancho || result.textura.alto  < aux_img.alto)
											result.textura.estirar(maxWidth, maxWidth).agregar_frame(aux_img.estirar(maxWidth, maxWidth));
										else result.textura.agregar_frame(aux_img.estirar(maxWidth, maxWidth));
									}
								}
							}
		                }
					}
					fclose(mtl);
					continue;
				}
				if(tamano >= 8) if(linea[0] == 'u' && linea[1] == 's' && linea[2] == 'e' && linea[3] == 'm' && linea[4] == 't' && linea[5] == 'l'){
					DynArray<char> compost(linea + 7);
					for(size_t i = 0; i < textures_names.size(); i++) if(textures_names[i] == compost){
						uv_idx = i;
						break;
					}
				}
			}
		}
		if(linea) delete[] linea;
		for(size_t i = 0; i < temp_vertices.size(); ++i) result.agregar_vertice(temp_vertices[i]);
		fclose(file);
		delete[] base_path;
		return result;
	}
	
	template<typename... Args> void hacer_al_salir(const Callback<Args...> doing){ if(stared->all_ready) stared->quake_exit = doing; }
	void cancelar_cierre(){ if( stared->all_ready) stared->cancel_clossing = true;  }
	void aceptar_cierre() { if(!stared->all_ready) stared->cancel_clossing = false; }
	
	img captura_de_ventana(){
	    img result(stared->iWidth, stared->iHeight);
	    // Definir rectángulo de bloqueo
		Gdiplus::Rect rect(0, 0, result.ancho, result.alto);
		// Datos de bloqueo
		Gdiplus::BitmapData data;
		stared->backBuffer->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &data);
		if(data.Stride == result.ancho * 4) for(size_t j=0,m=result.ancho*result.alto;j<m;j++)result.contenido[j]=((uint32_t*)data.Scan0)[j];
		stared->backBuffer->UnlockBits(&data);
	    return result;
	}
	
	template<typename Func, typename... Args> img& lienzo(img& hImage, Func fn, Args&&... args){
		if(!hImage.valido()) return hImage;
		
		uint32_t my_color = color();
		float    my_pen   = lapiz();
		const auto drawBuffer = [&](const size_t& y, const size_t& x_start, const size_t& x_end)->void{
			size_t offset = y * hImage.ancho;
			for(size_t i = x_start; i <= x_end; ++i) hImage.contenido[offset + i] = mezclar_colores(hImage.contenido[offset + i], my_color);
		};
		
		using namespace mini_traits;
		
		if_constexpr<sizeof...(args) == 0>([&](auto&&... unpackedArgs)->void{
			if(fn == (Func)maxiwin::borra){
				unsigned int maxItr = hImage.ancho * hImage.alto;
				for(unsigned int i = 0; i < maxItr; i++) hImage.contenido[i] = 0x00000000U;
			}
		}, args...);
		if_constexpr<sizeof...(args) == 1>([&](auto&&... unpackedArgs)->void{}, args...);
		if_constexpr<sizeof...(args) == 2>([&](auto&&... unpackedArgs)->void{
			auto a1 = GET_ARGUMENT(0, unpackedArgs...);
			auto a2 = GET_ARGUMENT(1, unpackedArgs...);
			if_constexpr<all_numerical_value<decltype(a1), decltype(a2)>>
			([&](const float& a1, const float& a2)->void{
				if(fn == (Func)maxiwin::punto){
					if(a1 == __clamp<float>(a1, 0, hImage.ancho - 1) && a2 == __clamp<float>(a2, 0, hImage.alto - 1))
						hImage.contenido[(unsigned int)a2 * hImage.ancho + (unsigned int)a1] = mezclar_colores(
						hImage.contenido[(unsigned int)a2 * hImage.ancho + (unsigned int)a1],
						my_color
					);
				}
				if(fn == (Func)maxiwin::balde){
					if(a1 != __clamp<float>(a1, 0, hImage.ancho - 1) || a2 != __clamp<float>(a2, 0, hImage.alto - 1)) return;
					uint32_t& targetValue = hImage.contenido[(unsigned int)a2 * hImage.ancho + (unsigned int)a1];
					if(my_color == targetValue) return;
					struct __balde__{
						static void exe(img& hImage, uint32_t& my_color, const int& x, const int& y, const uint32_t& targetValue){
							if(x < 0 || y < 0 || x >= hImage.ancho || y >= hImage.alto) return;
							uint32_t& currentColor = hImage.contenido[y * hImage.ancho + x];
							if(currentColor != targetValue) return;
							currentColor = mezclar_colores(currentColor, my_color);
							exe(hImage, my_color, x + 1, y, targetValue);
							exe(hImage, my_color, x - 1, y, targetValue);
							exe(hImage, my_color, x, y + 1, targetValue);
							exe(hImage, my_color, x, y - 1, targetValue);
						}
					}; __balde__::exe(hImage, my_color, (int)a1, (int)a2, targetValue);
				}
			}, a1, a2);
			/*if_constexpr<is_same_value<decltype(a1), decltype(a2), DynArray<float>>>
			([&](const DynArray<float>& a1, const DynArray<float>& a2)->void{
				if(fn == (Func)maxiwin::poligono_lleno){
					if(a1.size() != a2.size() || a1.size() < 3ULL) return;
					bool todos_alineados = true;
					for(size_t i = 2; i < a1.size() && todos_alineados; i++)
						if(int(a1[0] * (a2[1] - a2[i]) + a1[1] * (a2[i] - a2[0]) + a1[i] * (a2[0] - a2[1])) != 0) todos_alineados = false;
					if(todos_alineados) return;
					int ddx = hImage.ancho - 1;
					int minY = a2.top(),
						maxY = a2.top();
					float* first = (float*)a2.begin();
					while(++first != a2.end()){
						if(*first < minY) minY = *first;
						if(*first > maxY) maxY = *first;
					}
					// Scanline fill: por cada línea horizontal, buscar intersecciones con los lados
					for(int _y = minY; _y <= maxY; _y++){
						DynArray<int> nodos_x;
						for(size_t i = 0, j = a1.size() - 1; i < a1.size(); j = i++){
							int yi = a2[i], yj = a2[j],
								xi = a1[i], xj = a1[j];
							if((yi < _y && yj >= _y) || (yj < _y && yi >= _y)) nodos_x.push_back(xi + (_y - yi) * (xj - xi) / (yj - yi));
						}
						qsort(nodos_x.begin(), nodos_x.size(), sizeof(int), [](const void* a, const void* b){
							int arg1 = *(const int*)a;
						    int arg2 = *(const int*)b;
							return (arg1 > arg2) - (arg1 < arg2);
						});
						// Rellenar pares de intersecciones
						for(size_t k = 0; k + 1 < nodos_x.size(); k += 2){
							int x_start = nodos_x[k],
								x_end   = nodos_x[k + 1];
							if(x_start > x_end){
								int __k = x_start;
								x_start = x_end; x_end = __k;
							}
							if(x_start > ddx || x_end < 0) continue;
							x_start = __clamp(x_start, 0, ddx);
							x_end   = __clamp(x_end,   0, ddx);
							drawBuffer(_y, x_start, x_end);
						}
					}
				}
			}, a1, a2);*/
		}, args...);
		if_constexpr<sizeof...(args) == 3>([&](auto&&... unpackedArgs)->void{
			auto a1 = GET_ARGUMENT(0, unpackedArgs...);
			auto a2 = GET_ARGUMENT(1, unpackedArgs...);
			auto a3 = GET_ARGUMENT(2, unpackedArgs...);
			if_constexpr<all_numerical_value<decltype(a1), decltype(a2), decltype(a3)>>
			([&](const float& a1, const float& a2, const float& a3)->void{
				if(fn == (Func)maxiwin::circulo){
					if((int)a3 <= 0) return;
					my_pen /= 2;
					float outer_r = a3 + my_pen, inner_r = a3 - my_pen;
					inner_r = (inner_r < 0.0f) ? -inner_r : inner_r;
					float r2_outer = outer_r * outer_r,
						  r2_inner = inner_r * inner_r;
					int alto  = hImage.alto,
						ancho = hImage.ancho,
						ddx   = ancho - 1,
						ddy   = alto  - 1;
					// Acotar el área de escaneo
					int xmin = a1 - int(outer_r + 1.0f),
						xmax = a1 + int(outer_r + 1.0f),
						ymin = a2 - int(outer_r + 1.0f),
						ymax = a2 + int(outer_r + 1.0f);
					xmin = (xmin >   0) ? xmin :   0;
					xmax = (xmax < ddx) ? xmax : ddx;
					ymin = (ymin >   0) ? ymin :   0;
					ymax = (ymax < ddy) ? ymax : ddy;
					for(int y = ymin; y <= ymax; y++){
						int dy = y - a2;
						float dy2 = dy * dy;
						for(int x = xmin; x <= xmax; x++){
						    int dx = x - a1;
						    float d2 = float(dx * dx) + dy2;
						    if(d2 >= r2_inner && d2 <= r2_outer) hImage.contenido[y * ancho + x] = mezclar_colores(hImage.contenido[y * ancho + x], my_color);
						}
					}
				}
				if(fn == (Func)maxiwin::circulo_lleno){
					if((int)a3 == 1){
						if((int)a1 == __clamp<int>(a1, 0, hImage.ancho - 1) && (int)a2 == __clamp<int>(a2, 0, hImage.alto - 1))
							hImage.contenido[(int)a2 * hImage.ancho + (int)a1] = my_color;
						return;
					}
					int r2 = a3 * a3, alto = hImage.alto, ancho = hImage.ancho;
					int ddx = ancho - 1;
					for(int dy = -a3; dy <= a3; dy++){
						int y = a2 + dy;
						if(y < 0 || y >= alto) continue;
						int dx = square_root(r2 - dy * dy),
							x_start = a1 - dx,
							x_end   = a1 + dx;
						x_start = (x_start >   0) ? x_start :   0;
						x_end   = (x_end   < ddx) ? x_end   : ddx;
						drawBuffer(y, x_start, x_end);
					}
				}
			}, a1, a2, a3);
		}, args...);
		if_constexpr<sizeof...(args) == 4>([&](auto&&... unpackedArgs)->void{
			auto a1 = GET_ARGUMENT(0, unpackedArgs...);
			auto a2 = GET_ARGUMENT(1, unpackedArgs...);
			auto a3 = GET_ARGUMENT(2, unpackedArgs...);
			auto a4 = GET_ARGUMENT(3, unpackedArgs...);
			if_constexpr<all_numerical_value<decltype(a1), decltype(a2), decltype(a3), decltype(a4)>>
			([&](const float& a1, const float& a2, const float& a3, const float& a4)->void{
				if(fn == (Func)maxiwin::linea){
					my_pen /= 2.0f;
					if((int)my_pen == 0 && (int)a2 == (int)a4){
						ssize_t x_start = (((a1 < a3) ? a1 : a3) >           0LL) ? ((a1 < a3) ? a1 : a3) :             0LL,
								x_end   = (((a1 > a3) ? a1 : a3) <= hImage.ancho) ? ((a1 > a3) ? a1 : a3) : hImage.ancho -1;
						if(x_start >= hImage.ancho || x_end < 0LL) return;
						drawBuffer(a2, x_start, x_end);
						return;
					}
					pnt normal(-(a4 - a2), a3 - a1); normal /= square_root((a3 - a1) * (a3 - a1) + (a4 - a2) * (a4 - a2)); normal *= my_pen;
					pnt p1(a1, a2), p2(a1, a2), p3(a3, a4), p4(a3, a4);
					p1 += normal; p2 -= normal; p3 -= normal; p4 += normal;
					pnt centro((p1.x + p2.x + p3.x + p4.x) / 4.0f, (p1.y + p2.y + p3.y + p4.y) / 4.0f);
					pnt pts[4]{p1, p2, p3, p4}; for(int i=0;i<4;i++) pts[i] += pnt{0, 0, fast_atan2((pts + i)->y - centro.y, (pts + i)->x - centro.x)};
					qsort(pts, 4, sizeof(pnt), [](const void* a, const void* b){ return (((pnt*)a)->z > ((pnt*)b)->z) - (((pnt*)a)->z < ((pnt*)b)->z); });
					// Para cada línea horizontal entre el punto más alto y el más bajo
					float minY = pts[0].y,
						  maxY = pts[0].y;
					pnt* _first = pts;
					while(++_first != pts + 4){
						if(_first->y < minY) minY = _first->y;
						if(_first->y > maxY) maxY = _first->y;
					}
					ssize_t y_start = ceil__x(minY),
							y_end   = floor_x(maxY);
					y_start = (y_start > 0) ? y_start : 0;
					y_end   = (y_end < hImage.alto - 1) ? y_end : hImage.alto - 1;
					if(y_start >= hImage.alto || y_end < 0) return;
					for(int y = y_start; y <= y_end; y++){
						// Encontrar intersecciones del escaneo horizontal (y) con los bordes
						float inter_x[4]; int count = 0;
						for(int i=0;i<4;i++){
							pnt* a = pts + i;
							pnt* b = pts + ((i + 1) % 4);
							if((a->y <= y + 0.5f && b->y > y + 0.5f) || (b->y <= y + 0.5f && a->y > y + 0.5f)) inter_x[count++] = (a->y == b->y) ?
								a->x : (a->x + ((y + 0.5f) - a->y) * (b->x - a->x) / (b->y - a->y));
						}
						if(count < 2) continue;
						float x1 = inter_x[0],
							  x2 = inter_x[1];
						if(x1 > x2){
							float _k = x1;
							x1 = x2; x2 = _k;
						}
						ssize_t x_start = ceil__x(x1),
					    		x_end   = floor_x(x2);
						x_start = (x_start > 0) ? x_start : 0;
						x_end   = (x_end < hImage.ancho - 1) ? x_end : hImage.ancho - 1;
						if(x_start >= hImage.ancho || x_end < 0) continue;
						if(x_start == x_end) hImage.contenido[y * hImage.ancho + x_start] =
							mezclar_colores(hImage.contenido[y * hImage.ancho + x_start], my_color);
						else drawBuffer(y, x_start, x_end - !(x_end & 0x1));	
					}
				}
				if(fn == (Func)maxiwin::rectangulo){
					float half_pen = my_pen / 2.0f;
					int pen = round_x(my_pen);
					int ddx = hImage.ancho - 1,
						ddy = hImage.alto  - 1;
					int x1_in  = floor_x(((a1<a3)?a1:a3) +half_pen    -!((int)my_pen & 0x1)), x2_out = floor_x(((a1>a3)?a1:a3) +half_pen -!((int)my_pen & 0x1)),
						y1_in  = floor_x(((a2<a4)?a2:a4) +half_pen +1 -!((int)my_pen & 0x1)), y2_out = floor_x(((a2>a4)?a2:a4) +half_pen -!((int)my_pen & 0x1)),
						x2_in  = ceil__x(((a1>a3)?a1:a3) -half_pen),                          x1_out = ceil__x(((a1<a3)?a1:a3) -half_pen),
						y1_out = ceil__x(((a2<a4)?a2:a4) -half_pen),                          y2_in  = ceil__x(((a2>a4)?a2:a4) -half_pen -1);
					if(a1 == a3 || a2 == a4) return;
					if(x1_in >= x2_in || y1_in > y2_in){
						lienzo(hImage, maxiwin::rectangulo_lleno, x1_out, y1_out, x2_out + (my_pen != 1), y2_out + 1);
						return;
					}
					if(my_pen == 1){
						if(x1_out <= ddx && x2_out >= 0){
							if(y1_out >= 0 && y1_out <= ddy) drawBuffer(y1_out, (0>x1_out?0:x1_out), (ddx<x2_out?ddx:x2_out));
							if(y2_out >= 0 && y2_out <= ddy) drawBuffer(y2_out, (0>x1_out?0:x1_out), (ddx<x2_out?ddx:x2_out));
						}
						for(size_t y = (0>y1_in?0:y1_in), maxITR = (y2_in<ddy?y2_in:ddy); y <= maxITR; ++y){
							if(x1_out >= 0 && x1_out <= ddx) hImage.contenido[y * hImage.ancho + x1_out] =
								mezclar_colores(hImage.contenido[y * hImage.ancho + x1_out], my_color);
							if(x2_out >= 0 && x2_out <= ddx) hImage.contenido[y * hImage.ancho + x2_out] =
								mezclar_colores(hImage.contenido[y * hImage.ancho + x2_out], my_color);
						}
					}else{
						if(x1_out <= ddx && x2_out >= 0){
							for(size_t y = (0>y1_out?0:y1_out); y < (y1_in<ddy?y1_in:ddy); ++y) drawBuffer(y, (0>x1_out?0:x1_out), (ddx<x2_out?ddx:x2_out));
							for(size_t y = (0>(y2_in+1)?0:(y2_in + 1)); y <= (y2_out<ddy?y2_out:ddy); ++y) drawBuffer(y, (0>x1_out?0:x1_out), (ddx<x2_out?ddx:x2_out));
						}
						for(size_t y = (0>y1_in?0:y1_in), maxITR = (y2_in<ddy?y2_in:ddy); y <= maxITR; ++y){
							if(x1_out <= ddx && x1_in >= 0) drawBuffer(y, (0>x1_out?0:x1_out), (ddx<x1_in?ddx:x1_in));
							if(x2_in <= ddx && x2_out >= 0) drawBuffer(y, (0>x2_in?0:x2_in), (ddx<x2_out?ddx:x2_out));
						}
					}
				}
				if(fn == (Func)maxiwin::rectangulo_lleno){
					int ddx = hImage.ancho - 1;
					int ddy = hImage.alto  - 1;
					int x1 = (a1 < a3) ? a1 : a3;
					int x2 = (a1 > a3) ? a1 : a3;
					int y1 = (a2 < a4) ? a2 : a4;
					int y2 = (a2 > a4) ? a2 : a4;
					if(x2 < 0 || x1 > ddx || y2 < 0 || y1 > ddy) return;
					int startY = (y1 > 0) ? y1 : 0;
					int endY   = (y2 < ddy) ? y2 : ddy;
					int startX = (x1 > 0) ? x1 : 0;
					int endX   = (x2 < ddx) ? x2 : ddx;
					for(int i = startY; i < endY; ++i) drawBuffer(i, startX, endX);
				}
			}, a1, a2, a3, a4);
		}, args...);
		if_constexpr<sizeof...(args) == 5>([&](auto&&... unpackedArgs)->void{
			auto a1 = GET_ARGUMENT(0, unpackedArgs...);
			auto a2 = GET_ARGUMENT(1, unpackedArgs...);
			auto a3 = GET_ARGUMENT(2, unpackedArgs...);
			auto a4 = GET_ARGUMENT(3, unpackedArgs...);
			auto a5 = GET_ARGUMENT(4, unpackedArgs...);
			if_constexpr<all_numerical_value<decltype(a1), decltype(a2), decltype(a3), decltype(a4), decltype(a5)>>
			([&](const float& a1, const float& a2, const float& a3, const float& a4, const float& a5)->void{
				if(fn == (Func)maxiwin::elipse){
					if(a3 <= 0.0f) return;
					{
						float _k = a4;
						a4 = a5; a5 = _k;
					}
					float radio_ext = a3 + my_pen / 2.0f;
					float radio_int = a3 - my_pen / 2.0f;
					float r2_ext = radio_ext * radio_ext;
					float r2_int = radio_int * radio_int;
					float a_ini = a4 - 360.0f *int(a4 / 360.0f);
					float a_fin = a5 - 360.0f *int(a5 / 360.0f);
					if(a_ini < 0.0f) a_ini += 360.0f;
					if(a_fin < 0.0f) a_fin += 360.0f;
					bool angulo_inverso = a_ini > a_fin;
					int ancho = hImage.ancho, alto = hImage.alto;
					int ddx = ancho - 1, ddy = alto - 1;
					for(int y = int(a2 - radio_ext); y <= int(a2 + radio_ext); y++){
						if(y < 0 || y > ddy) continue;
						float dy = y - a2;
						float dy2 = dy * dy;
						int dx_max = square_root(r2_ext - dy2);
						int x_min = __clamp(int(a1 - dx_max), 0, ddx);
						int x_max = __clamp(int(a1 + dx_max), 0, ddx);
						ssize_t x_start = -1;
						for(int x = x_min; x <= x_max; x++){
							float dx = x - a1;
							float d2 = dx * dx + dy2;
							if(d2 > r2_ext || d2 < r2_int){
								if(x_start != -1){
									drawBuffer(y, x_start, x);
								    x_start = -1;
								}
								continue;
							}
							float ang = M_RAD2DEG(fast_atan2(-dy, dx));
							if(ang < 0.0f) ang += 360.0f;
							bool dentroSector = (!angulo_inverso&&ang>=a_ini&&ang<=a_fin)||(angulo_inverso&&(ang>=a_ini||ang<=a_fin));
							if(dentroSector){ if(x_start == -1) x_start = x; }
							else{
								if(x_start != -1){
									drawBuffer(y, x_start, x);
								    x_start = -1;
								}
							}
						}
						if(x_start != -1) drawBuffer(y, x_start, x_max);
					}
				}
				if(fn == (Func)maxiwin::elipse_lleno){
					if(a3 <= 0.0f) return;
					{
						float _k = a4;
						a4 = a5; a5 = _k;
					}
					float radioCuadrado = a3 * a3;
					float angInicio = a4 - 360.0f * int(a4 / 360.0f),
					      angFinal  = a5 - 360.0f * int(a5 / 360.0f);
					if(angInicio < 0.0f) angInicio += 360.0f;
					if(angFinal  < 0.0f) angFinal  += 360.0f;
					bool anguloInverso = angInicio > angFinal;
					int maxX  = hImage.ancho - 1;
					int maxY  = hImage.alto  - 1;
					for(int y = int(a2 - a3); y <= int(a2 + a3); ++y){
					    if(y < 0 || y > maxY) continue;
					    float dy = y - a2;
					    float dy2 = dy * dy;
						if(dy2 > radioCuadrado) continue;
						int dxMax = square_root(radioCuadrado - dy2);
						int xMin = __clamp<int>(a1 - dxMax, 0, maxX);
						int xMax = __clamp<int>(a1 + dxMax, 0, maxX);
						int xInicio = -1;
						for(int x = xMin; x <= xMax; x++){
							float dx = x - a1;
							float distancia2 = dx * dx + dy2;
							if(distancia2 > radioCuadrado) continue;
							float angulo = M_RAD2DEG(fast_atan2(-dy, dx)); // Inversión Y
							if(angulo < 0.0f) angulo += 360.0f;
							bool dentroDelSector = (!anguloInverso&&angulo>=angInicio&&angulo<=angFinal)||(anguloInverso&&(angulo>=angInicio||angulo<=angFinal));
							if(dentroDelSector && xInicio == -1) xInicio = x; else if(xInicio != -1){
								drawBuffer(y, xInicio, x);
							    xInicio = -1;
							}
						}
						if(xInicio != -1) drawBuffer(y, xInicio, xMax);
					}
				}
			}, a1, a2, a3, a4, a5);
		}, args...);
		if_constexpr<sizeof...(args) == 6>([&](auto&&... unpackedArgs)->void{
			auto a1 = GET_ARGUMENT(0, unpackedArgs...);
			auto a2 = GET_ARGUMENT(1, unpackedArgs...);
			auto a3 = GET_ARGUMENT(2, unpackedArgs...);
			auto a4 = GET_ARGUMENT(3, unpackedArgs...);
			auto a5 = GET_ARGUMENT(4, unpackedArgs...);
			auto a6 = GET_ARGUMENT(5, unpackedArgs...);
			if_constexpr<all_numerical_value<decltype(a1), decltype(a2), decltype(a3), decltype(a4), decltype(a5), decltype(a6)>>
			([&](const float& a1, const float& a2, const float& a3, const float& a4, const float& a5, const float& a6)->void{
				if(fn == (Func)maxiwin::arco){
					float A1 = (float)a3 - (float)a1, B1 = (float)a4 - (float)a2,
					      A2 = (float)a5 - (float)a3, B2 = (float)a6 - (float)a4,
						  C1 = (A1 * ((float)a1 + (float)a3) + B1 * ((float)a2 + (float)a4)) / 2.0f,
						  C2 = (A2 * ((float)a3 + (float)a5) + B2 * ((float)a4 + (float)a6)) / 2.0f;
					float D = A1 * B2 - A2 * B1;
					if((D < 0 ? -D : D) < 1e-5) return; // puntos colineales
					float cx = (C1 * B2 - C2 * B1) / D,
					      cy = (A1 * C2 - A2 * C1) / D;
					float dx = (float)a1 - cx, dy = (float)a2 - cy,
					      radio = square_root(dx*dx + dy*dy);
					float _a1 = M_RAD2DEG(fast_atan2((float)a2 - cy, (float)a1 - cx)),
						  _a2 = M_RAD2DEG(fast_atan2((float)a4 - cy, (float)a3 - cx)),
						  _a3 = M_RAD2DEG(fast_atan2((float)a6 - cy, (float)a5 - cx));
					while(_a2 < _a1) _a2 += 360;
					while(_a3 < _a1) _a3 += 360;
					_a1 = -_a1; _a3 = -_a3;
					if((A1 == 0.0f ? 0.0f : B1 / A1) <= (A2 == 0.0f ? 0.0f : B2 / A2)){
						float _k = _a1;
						_a1 = _a3; _a3 = _k;
					}
					////
					if(radio <= 0.0f) return;
					float radio_ext = radio + my_pen / 2.0f;
					float radio_int = radio - my_pen / 2.0f;
					float r2_ext = radio_ext * radio_ext;
					float r2_int = radio_int * radio_int;
					float a_ini = _a1 - 360.0f *int(_a1 / 360.0f);
					float a_fin = _a3 - 360.0f *int(_a3 / 360.0f);
					if(a_ini < 0.0f) a_ini += 360.0f;
					if(a_fin < 0.0f) a_fin += 360.0f;
					bool angulo_inverso = a_ini > a_fin;
					int ancho = hImage.ancho, alto = hImage.alto;
					int ddx = ancho - 1, ddy = alto - 1;
					for(int y = int(cy - radio_ext); y <= int(cy + radio_ext); y++){
						if(y < 0 || y > ddy) continue;
						float dy = y - cy;
						float dy2 = dy * dy;
						int dx_max = square_root(r2_ext - dy2);
						int x_min = __clamp(int(cx - dx_max), 0, ddx);
						int x_max = __clamp(int(cx + dx_max), 0, ddx);
						ssize_t x_start = -1;
						for(int x = x_min; x <= x_max; x++){
							float dx = x - cx;
							float d2 = dx * dx + dy2;
							if(d2 > r2_ext || d2 < r2_int){
								if(x_start != -1){
									drawBuffer(y, x_start, x);
								    x_start = -1;
								}
								continue;
							}
							float ang = M_RAD2DEG(fast_atan2(-dy, dx));
							if(ang < 0.0f) ang += 360.0f;
							bool dentroSector = (!angulo_inverso&&ang>=a_ini&&ang<=a_fin)||(angulo_inverso&&(ang>=a_ini||ang<=a_fin));
							if(dentroSector){ if(x_start == -1) x_start = x; }
							else{
								if(x_start != -1){
									drawBuffer(y, x_start, x);
								    x_start = -1;
								}
							}
						}
						if(x_start != -1) drawBuffer(y, x_start, x_max);
					}
				}
				if(fn == (Func)maxiwin::arco_lleno){
					float A1 = (float)a3 - (float)a1, B1 = (float)a4 - (float)a2,
					      A2 = (float)a5 - (float)a3, B2 = (float)a6 - (float)a4,
						  C1 = (A1 * ((float)a1 + (float)a3) + B1 * ((float)a2 + (float)a4)) / 2.0f,
						  C2 = (A2 * ((float)a3 + (float)a5) + B2 * ((float)a4 + (float)a6)) / 2.0f;
					float D = A1 * B2 - A2 * B1;
					if((D < 0 ? -D : D) < 1e-5) return; // puntos colineales
					float cx = (C1 * B2 - C2 * B1) / D,
					      cy = (A1 * C2 - A2 * C1) / D;
					float dx = (float)a1 - cx, dy = (float)a2 - cy,
					      radio = square_root(dx*dx + dy*dy);
					float _a1 = M_RAD2DEG(fast_atan2((float)a2 - cy, (float)a1 - cx)),
						  _a2 = M_RAD2DEG(fast_atan2((float)a4 - cy, (float)a3 - cx)),
						  _a3 = M_RAD2DEG(fast_atan2((float)a6 - cy, (float)a5 - cx));
					while(_a2 < _a1) _a2 += 360;
					while(_a3 < _a1) _a3 += 360;
					_a1 = -_a1; _a3 = -_a3;
					if((A1 == 0.0f ? 0.0f : B1 / A1) <= (A2 == 0.0f ? 0.0f : B2 / A2)){
						float _k = _a1;
						_a1 = _a3; _a3 = _k;
					}
					/////
					if(radio <= 0.0f) return;
					float radioCuadrado = radio * radio;
					float angInicio = a1 - 360.0f * int(a1 / 360.0f),
					      angFinal  = a3 - 360.0f * int(a3 / 360.0f);
					if(angInicio < 0.0f) angInicio += 360.0f;
					if(angFinal  < 0.0f) angFinal  += 360.0f;
					bool anguloInverso = angInicio > angFinal;
					int maxX  = hImage.ancho - 1;
					int maxY  = hImage.alto  - 1;
					for(int y = int(cy - radio); y <= int(cy + radio); ++y){
					    if(y < 0 || y > maxY) continue;
					    float dy = y - cy;
					    float dy2 = dy * dy;
						if(dy2 > radioCuadrado) continue;
						int dxMax = square_root(radioCuadrado - dy2);
						int xMin = __clamp<int>(cx - dxMax, 0, maxX);
						int xMax = __clamp<int>(cx + dxMax, 0, maxX);
						int xInicio = -1;
						for(int x = xMin; x <= xMax; x++){
							float dx = x - cx;
							float distancia2 = dx * dx + dy2;
							if(distancia2 > radioCuadrado) continue;
							float angulo = M_RAD2DEG(fast_atan2(-dy, dx)); // Inversión Y
							if(angulo < 0.0f) angulo += 360.0f;
							bool dentroDelSector = (!anguloInverso&&angulo>=angInicio&&angulo<=angFinal)||(anguloInverso&&(angulo>=angInicio||angulo<=angFinal));
							if(dentroDelSector && xInicio == -1) xInicio = x; else if(xInicio != -1){
								drawBuffer(y, xInicio, x);
							    xInicio = -1;
							}
						}
						if(xInicio != -1) drawBuffer(y, xInicio, xMax);
					}
				}
				if(fn == (Func)maxiwin::triangulo_lleno){
					if((float)a1*((float)a4 - (float)a6) + (float)a3*((float)a6 - (float)a2) + (float)a5*((float)a2 - (float)a4) == 0) return; // Area = 0
					int ddx = hImage.ancho - 1;
					pnt pts[3]{{(float)a1, (float)a2, 0}, {(float)a3, (float)a4, 0}, {(float)a5, (float)a6, 0}};
					qsort(pts, 3, sizeof(pnt), [](const void* a, const void* b){ return (((pnt*)a)->y > ((pnt*)b)->y) - (((pnt*)a)->y < ((pnt*)b)->y); });
					int clamp_A = __clamp<int>(pts[0].y, 0, hImage.alto - 1),
						clamp_B = __clamp<int>(pts[1].y, 0, hImage.alto - 1),
						clamp_C = __clamp<int>(pts[2].y, 0, hImage.alto - 1);
					auto interp = [](int x0, int y0, int a1, int a2, int y) -> int{
						if((int)a2 == y0) return x0;
						return x0 + (a1 - x0) * (y - y0) / (a2 - y0);
					};
					for(int y = clamp_A; y <= clamp_B; ++y){
						int x_start = interp(pts[0].x, pts[0].y, pts[2].x, pts[2].y, y),
							x_end   = interp(pts[0].x, pts[0].y, pts[1].x, pts[1].y, y);
						if(x_start > x_end){
							int __k = x_start;
							x_start = x_end; x_end = __k;
						}
						if(x_start > ddx || x_end < 0) continue;
						x_start = __clamp(x_start, 0, ddx);
						x_end   = __clamp(x_end,   0, ddx);
						drawBuffer(y, x_start, x_end);
					}
					for(int y = clamp_B + 1; y <= clamp_C; ++y){
						int x_start = interp(pts[0].x, pts[0].y, pts[2].x, pts[2].y, y),
							x_end   = interp(pts[1].x, pts[1].y, pts[2].x, pts[2].y, y);
						if(x_start > x_end){
							int __k = x_start;
							x_start = x_end; x_end = __k;
						}
						if(x_start > ddx || x_end < 0) continue;
						x_start = __clamp(x_start, 0, ddx);
						x_end   = __clamp(x_end,   0, ddx);
						drawBuffer(y, x_start, x_end);
					}
				}
			}, a1, a2, a3, a4, a5, a6);
		}, args...);
		
		return hImage;
	}
	#define _MAXIWIN_H__poligono_lleno_ decltype(static_cast<void(*)(DynArray<float>,DynArray<float>)>(maxiwin::poligono_lleno))
	img& lienzo(img& hImage, _MAXIWIN_H__poligono_lleno_, const DynArray<float> a1, const DynArray<float> a2){
		if(!hImage.valido()) return hImage;
		
		uint32_t my_color = color();
		float    my_pen   = lapiz();
		const auto drawBuffer = [&](const size_t& y, const size_t& x_start, const size_t& x_end)->void{
			size_t offset = y * hImage.ancho;
			for(size_t i = x_start; i <= x_end; ++i) hImage.contenido[offset + i] = mezclar_colores(hImage.contenido[offset + i], my_color);
		};
		
		if(a1.size() != a2.size() || a1.size() < 3ULL) return hImage;
		bool todos_alineados = true;
		for(size_t i = 2; i < a1.size() && todos_alineados; i++)
			if(int(a1[0] * (a2[1] - a2[i]) + a1[1] * (a2[i] - a2[0]) + a1[i] * (a2[0] - a2[1])) != 0) todos_alineados = false;
		if(todos_alineados) return hImage;
		int ddx = hImage.ancho - 1;
		int minY = a2.top(),
			maxY = a2.top();
		float* first = (float*)a2.begin();
		while(++first != a2.end()){
			if(*first < minY) minY = *first;
			if(*first > maxY) maxY = *first;
		}
		// Scanline fill: por cada línea horizontal, buscar intersecciones con los lados
		for(int _y = minY; _y <= maxY; _y++){
			DynArray<int> nodos_x;
			for(size_t i = 0, j = a1.size() - 1; i < a1.size(); j = i++){
				int yi = a2[i], yj = a2[j],
					xi = a1[i], xj = a1[j];
				if((yi < _y && yj >= _y) || (yj < _y && yi >= _y)) nodos_x.push_back(xi + (_y - yi) * (xj - xi) / (yj - yi));
			}
			qsort(nodos_x.begin(), nodos_x.size(), sizeof(int), [](const void* a, const void* b){
				int arg1 = *(const int*)a;
			    int arg2 = *(const int*)b;
				return (arg1 > arg2) - (arg1 < arg2);
			});
			// Rellenar pares de intersecciones
			for(size_t k = 0; k + 1 < nodos_x.size(); k += 2){
				int x_start = nodos_x[k],
					x_end   = nodos_x[k + 1];
					if(x_start > x_end){
					int __k = x_start;
					x_start = x_end; x_end = __k;
				}
				if(x_start > ddx || x_end < 0) continue;
				x_start = __clamp(x_start, 0, ddx);
				x_end   = __clamp(x_end,   0, ddx);
				drawBuffer(_y, x_start, x_end);
			}
		}
		
		return hImage;
	}
	template<size_t N> img& lienzo(img& hImage, _MAXIWIN_H__poligono_lleno_, const float(&_x)[N],const float(&_y)[N])
	{return lienzo(hImage, poligono_lleno, DynArray<float>(_x), DynArray<float>(_y));}
	template<typename... Args> img& lienzo(img& hImage, _MAXIWIN_H__imagen_, Args&&... args){
		if(!hImage.valido()) return hImage;
		
		uint32_t my_color = color();
		float    my_pen   = lapiz();
		
		using namespace mini_traits;
		
		if_constexpr<sizeof...(args) == 1>([&](auto&&... unpackedArgs)->void{
			auto a1 = GET_ARGUMENT(0, unpackedArgs...);
			if_constexpr<is_same_cvref_type<decltype(a1), maxiwin::pnt>>
			([&](const maxiwin::pnt& a1)->void{
				size_t wW = vancho(),
					   wH = valto();
				float size = (wW > wH ? wW : wH) / 200.0f;
				size = size > 1.0f ? size : 1.0f;
				lienzo(hImage, maxiwin::circulo_lleno, a1.x, a1.y, size);
			}, a1);
		}, args...);
		if_constexpr<sizeof...(args) == 3>([&](auto&&... unpackedArgs)->void{
			auto a1 = GET_ARGUMENT(0, unpackedArgs...);
			auto a2 = GET_ARGUMENT(1, unpackedArgs...);
			auto a3 = GET_ARGUMENT(2, unpackedArgs...);
			if_constexpr<all_numerical_value<decltype(a1), decltype(a2)>>
			([&](const float& a1, const float& a2)->void{
				if_constexpr<is_same_cvref_type<decltype(a3), maxiwin::img>>
				([&](const maxiwin::img& a3)->void{
					ssize_t x = a1,
							y = a2;
					size_t x0 = (0 > x ? 0 : x),
						   y0 = (0 > y ? 0 : y),
						   x1 = (x + a3.ancho < hImage.ancho ? x + a3.ancho : hImage.ancho),
						   y1 = (y + a3.alto  < hImage.alto  ? y + a3.alto  : hImage.alto );
					for(size_t i = y0; i < y1; i++){
					    size_t offA = i * hImage.ancho;
					    size_t offB = (i - y) * a3.ancho;
					    for(size_t j = x0; j < x1; j++) hImage.contenido[offA + j] = mezclar_colores(
					        hImage.contenido[offA + j],
					        a3.contenido[offB + (j - x)]
					    );
					}
				}, a3);
				if_constexpr<is_same_cvref_type<decltype(a3), DynArray<char>>>
				([&](const DynArray<char>& a3)->void{ lienzo(hImage, imagen, a1, a2, cargar_imagen(a3)); }, a3);
				if_constexpr<is_same_cvref_type<decltype(a3), maxiwin::anim>>
				([&](maxiwin::anim& a3)->void{
					lienzo(hImage, imagen, a1, a2, a3.obtener_frame());
					a3.siguiente_frame();
				}, a3);
			}, a1, a2);
		}, args...);
		
		return hImage;
	}
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
	switch(message){
		case WM_SIZE:{
		    RECT R;
		    GetClientRect(hWnd, &R);
		    int w = LOWORD(lParam);
		    int h = HIWORD(lParam);
			// Al minimizar envia WM_SIZE(0,0)
		    if(w == 0 && h == 0) break;
		    if(w != maxiwin::stared->iWidth || h != maxiwin::stared->iHeight) maxiwin::stared->newMemDC(w, h);
		    maxiwin::stared->iWidth = w;
		    maxiwin::stared->iHeight = h;
	    break;}
		case WM_SIZING:{
		    RECT* pRECT = (RECT*)lParam;
		    int w, h;
		    maxiwin::frame_real(maxiwin::stared->iWidth, maxiwin::stared->iHeight, w, h);
		    switch (wParam){
		    	case WMSZ_BOTTOM:	   pRECT->bottom = pRECT->top + h;								   break;
		    	case WMSZ_TOP:		   pRECT->top = pRECT->bottom - h;								   break;
		    	case WMSZ_RIGHT:  	   pRECT->right = pRECT->left + w;								   break;
			    case WMSZ_LEFT:		   pRECT->left = pRECT->right - w;								   break;
		    	case WMSZ_TOPLEFT:	   pRECT->top = pRECT->bottom - h; pRECT->left = pRECT->right - w; break;
		    	case WMSZ_TOPRIGHT:    pRECT->top = pRECT->bottom - h; pRECT->right = pRECT->left + w; break;
		    	case WMSZ_BOTTOMLEFT:  pRECT->bottom = pRECT->top + h; pRECT->left = pRECT->right - w; break;
		    	case WMSZ_BOTTOMRIGHT: pRECT->bottom = pRECT->top + h; pRECT->right = pRECT->left + w; break;
		    }
		return TRUE;}
		case WM_PAINT:{
		    PAINTSTRUCT ps;
		    HDC hdc = BeginPaint(hWnd, &ps);
		    Gdiplus::Graphics gWindow(hdc);
		    gWindow.SetCompositingMode(Gdiplus::CompositingModeSourceOver); // Habilita transparencia
		    Gdiplus::SolidBrush myBrush(Gdiplus::Color(255,0,0,0));
		    gWindow.FillRectangle(&myBrush,0,0,maxiwin::stared->iWidth,maxiwin::stared->iHeight);
		    if(maxiwin::stared->backBuffer && maxiwin::stared->grafics) gWindow.DrawImage(maxiwin::stared->backBuffer, 0, 0);
		    EndPaint(hWnd, &ps);
	    break;}
		case WM_MOUSEMOVE:{
		    maxiwin::stared->_raton_dentro = true;
		    maxiwin::stared->_xraton = LOWORD(lParam);
		    maxiwin::stared->_yraton = HIWORD(lParam);
		    maxiwin::stared->_bot_izq = wParam & MK_LBUTTON;
		    maxiwin::stared->_bot_der = wParam & MK_RBUTTON;
	    break;}
		case WM_MOUSELEAVE:{ maxiwin::stared->_raton_dentro = false; break; }
		case WM_LBUTTONDOWN:{ maxiwin::stared->_bot_izq = true;  break; }
		case WM_LBUTTONUP:  { maxiwin::stared->_bot_izq = false; break; }
		case WM_RBUTTONDOWN:{ maxiwin::stared->_bot_der = true;  break; }
		case WM_RBUTTONUP:  { maxiwin::stared->_bot_der = false; break; }
		case WM_KEYDOWN:{ maxiwin::stared->__teclas_actualizar(wParam, true);  break; }
	    case WM_KEYUP:  { maxiwin::stared->__teclas_actualizar(wParam, false); break; }
	    case WM_CLOSE:{
	    	if(maxiwin::stared->cancel_clossing){
	    		maxiwin::stared->cancel_clossing = false;
	    		if(maxiwin::stared->quake_exit.get()) maxiwin::stared->quake_exit();
	    		return 0;
			}
		break;}
		case WM_DESTROY:{
			if(maxiwin::stared->quake_exit.get()){
		        maxiwin::stared->quake_exit();
				maxiwin::stared->quake_exit.set(nullptr);
			}
	        PostQuitMessage(0);
	        maxiwin::stared->all_ready = false;
	        exit(0);
	    	return 0;
		}
	}
    return DefWindowProc(hWnd, message, wParam, lParam);
}
#else
#error "MaxiWin ya no es el novio de Miniwin"
#endif