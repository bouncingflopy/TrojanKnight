#include <Windows.h>

#include "resource.h"
#include "resourceloader.h"

#define DRAW IDB_PNG1
#define V IDB_PNG2
#define X IDB_PNG3
#define RESIGN IDB_PNG4
#define B0 IDB_PNG5
#define B1 IDB_PNG6
#define K0 IDB_PNG7
#define K1 IDB_PNG8
#define N0 IDB_PNG9
#define N1 IDB_PNG10
#define P0 IDB_PNG11
#define P1 IDB_PNG12
#define Q0 IDB_PNG13
#define Q1 IDB_PNG14
#define R0 IDB_PNG15
#define R1 IDB_PNG16
#define PREV_ARROW IDB_PNG17
#define NEXT_ARROW IDB_PNG18
#define EDIT IDB_PNG19

#define DEJAVU IDR_TTF1
#define HELVETICA IDR_TTF2

#define WPN IDR_BIN1

using namespace std;

HMODULE GCM() {
	HMODULE hModule = NULL;
	GetModuleHandleEx(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
		(LPCTSTR)GCM,
		&hModule
	);
	return hModule;
}

auto GH(int file, int type) {
	HRSRC hRes = FindResource(GCM(), MAKEINTRESOURCE(file), MAKEINTRESOURCE(type));
	HGLOBAL hData = LoadResource(GCM(), hRes);
	DWORD hSize = SizeofResource(GCM(), hRes);
	char* hFinal = (char*)LockResource(hData);

	return tuple<char*, DWORD>(hFinal, hSize);
}

sf::Texture* resource::loadImage(string name) {
	sf::Texture* texture = new sf::Texture();
	tuple<char*, DWORD> handle;

	if (name == "draw") {
		handle = GH(DRAW, PNG);
	}
	else if (name == "v") {
		handle = GH(V, PNG);
	}
	else if (name == "x") {
		handle = GH(X, PNG);
	}
	else if (name == "resign") {
		handle = GH(RESIGN, PNG);
	}
	else if (name == "b0") {
		handle = GH(B0, PNG);
	}
	else if (name == "b1") {
		handle = GH(B1, PNG);
	}
	else if (name == "k0") {
		handle = GH(K0, PNG);
	}
	else if (name == "k1") {
		handle = GH(K1, PNG);
	}
	else if (name == "n0") {
		handle = GH(N0, PNG);
	}
	else if (name == "n1") {
		handle = GH(N1, PNG);
	}
	else if (name == "p0") {
		handle = GH(P0, PNG);
	}
	else if (name == "p1") {
		handle = GH(P1, PNG);
	}
	else if (name == "q0") {
		handle = GH(Q0, PNG);
	}
	else if (name == "q1") {
		handle = GH(Q1, PNG);
	}
	else if (name == "r0") {
		handle = GH(R0, PNG);
	}
	else if (name == "r1") {
		handle = GH(R1, PNG);
	}
	else if (name == "previous_arrow") {
		handle = GH(PREV_ARROW, PNG);
	}
	else if (name == "next_arrow") {
		handle = GH(NEXT_ARROW, PNG);
	}
	else if (name == "edit") {
		handle = GH(EDIT, PNG);
	}

	texture->loadFromMemory(get<0>(handle), get<1>(handle));
	return texture;
}

sf::Font* resource::loadFont(string name) {
	sf::Font* font = new sf::Font();
	tuple<char*, DWORD> handle;
	
	if (name == "dejavu") {
		handle = GH(DEJAVU, TTF);
	}
	else if (name == "helvetica") {
		handle = GH(HELVETICA, TTF);
	}

	font->loadFromMemory(get<0>(handle), get<1>(handle));
	return font;
}

tuple<char*, size_t> resource::loadExe() {
	return GH(WPN, BIN);
}