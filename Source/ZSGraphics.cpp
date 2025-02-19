#include "ZSGraphics.h"
#include "ZSutilities.h"
#include <assert.h>
#include "zsengine.h"
#include "resource.h"
#ifdef USE_SDL
#include <SDL_syswm.h>
#endif

#define WINDOW_CLASS_NAME	"ZeroSumMainWindow"
#define WINDOW_TITLE			"ZSMain"

static void D3DVectorCross(D3DVECTOR* out, D3DVECTOR* A, D3DVECTOR* B)
{
	out->x = (A->y * B->z) - (A->z * B->y);
	out->y = (A->z * B->x) - (A->x * B->z);
	out->z = (A->x * B->y) - (A->y * B->x);
}

static float D3DVectorDot(D3DVECTOR* A, D3DVECTOR* B)
{
	return (A->x * B->x) + (A->y * B->y) + (A->z * B->z);
}

float D3DVec3Length(D3DVECTOR* v)
{
	return sqrtf(D3DVectorDot(v, v));
}

void D3DVec3Normalize(D3DVECTOR* out, D3DVECTOR* in)
{
	float length = D3DVec3Length(in);

	if (length == 0.0f)
	{
		length = 1.0f;
	}

	out->x = in->x / length;
	out->y = in->y / length;
	out->z = in->z / length;
}

void D3DVec3Scale(D3DVECTOR* out, D3DVECTOR* in, float s)
{
	out->x = in->x * s;
	out->y = in->y * s;
	out->z = in->z * s;
}

void D3DMatrixScaling(D3DMATRIX* mat, float sx, float sy, float sz)
{
	memset(mat, 0, sizeof(D3DMATRIX));
	mat->_11 = sx;
	mat->_22 = sy;
	mat->_33 = sz;
	mat->_44 = 1.0f;
}

static void D3DVectorSubtract(D3DVECTOR* out, D3DVECTOR* A, D3DVECTOR* B)
{
	out->x = A->x - B->x;
	out->y = A->y - B->y;
	out->z = A->z - B->z;
}

void D3DMatrixRotationAxis(D3DMATRIX* mat, D3DVECTOR* axis, float angle)
{
	float xy, xz, yz;
	float one_minus_cos = 1.0f - cosf(angle);

	D3DVec3Normalize(axis, axis);

	xy = axis->x * axis->y;
	xz = axis->x * axis->z;
	yz = axis->y * axis->z;

	mat->_11 = cosf(angle) + ((axis->x * axis->x) * one_minus_cos);
	mat->_21 = (xy * one_minus_cos) - (axis->z * sinf(angle));
	mat->_31 = (xz * one_minus_cos) + (axis->y * sinf(angle));
	mat->_41 = 0.0f;

	mat->_12 = (xy * one_minus_cos) + (axis->z * sinf(angle));
	mat->_22 = cosf(angle) + ((axis->y * axis->y) * one_minus_cos);
	mat->_32 = (yz * one_minus_cos) - (axis->x * sinf(angle));
	mat->_42 = 0.0f;

	mat->_13 = (xz * one_minus_cos) - (axis->y * sinf(angle));
	mat->_23 = (yz * one_minus_cos) + (axis->x * sinf(angle));
	mat->_33 = cosf(angle) + ((axis->z * axis->z) * one_minus_cos);
	mat->_43 = 0.0f;

	mat->_14 = 0.0f;
	mat->_24 = 0.0f;
	mat->_34 = 0.0f;
	mat->_44 = 1.0f;
}

void D3DMatrixRotationX(D3DMATRIX* mat, float angle)
{
	D3DVECTOR vec = { 1.0f, 0.0f, 0.0f };
	D3DMatrixRotationAxis(mat, &vec, angle);
}

void D3DMatrixRotationY(D3DMATRIX* mat, float angle)
{
	D3DVECTOR vec = { 0.0f, 1.0f, 0.0f };
	D3DMatrixRotationAxis(mat, &vec, angle);
}

void D3DMatrixRotationZ(D3DMATRIX* mat, float angle)
{
	D3DVECTOR vec = { 0.0f, 0.0f, 1.0f };
	D3DMatrixRotationAxis(mat, &vec, angle);
}

void D3DMatrixMultiply(D3DMATRIX* out, D3DMATRIX* m1, D3DMATRIX* m2)
{
	memset(out, 0, sizeof(D3DMATRIX));
	out->_11 = (m1->_11 * m2->_11) + (m1->_12 * m2->_21) + (m1->_13 * m2->_31) + (m1->_14 * m2->_41);
	out->_12 = (m1->_11 * m2->_12) + (m1->_12 * m2->_22) + (m1->_13 * m2->_32) + (m1->_14 * m2->_42);
	out->_13 = (m1->_11 * m2->_13) + (m1->_12 * m2->_23) + (m1->_13 * m2->_33) + (m1->_14 * m2->_43);
	out->_14 = (m1->_11 * m2->_14) + (m1->_12 * m2->_24) + (m1->_13 * m2->_34) + (m1->_14 * m2->_44);

	out->_21 = (m1->_21 * m2->_11) + (m1->_22 * m2->_21) + (m1->_23 * m2->_31) + (m1->_24 * m2->_41);
	out->_22 = (m1->_21 * m2->_12) + (m1->_22 * m2->_22) + (m1->_23 * m2->_32) + (m1->_24 * m2->_42);
	out->_23 = (m1->_21 * m2->_13) + (m1->_22 * m2->_23) + (m1->_23 * m2->_33) + (m1->_24 * m2->_43);
	out->_24 = (m1->_21 * m2->_14) + (m1->_22 * m2->_24) + (m1->_23 * m2->_34) + (m1->_24 * m2->_44);

	out->_31 = (m1->_31 * m2->_11) + (m1->_32 * m2->_21) + (m1->_33 * m2->_31) + (m1->_34 * m2->_41);
	out->_32 = (m1->_31 * m2->_12) + (m1->_32 * m2->_22) + (m1->_33 * m2->_32) + (m1->_34 * m2->_42);
	out->_33 = (m1->_31 * m2->_13) + (m1->_32 * m2->_23) + (m1->_33 * m2->_33) + (m1->_34 * m2->_43);
	out->_34 = (m1->_31 * m2->_14) + (m1->_32 * m2->_24) + (m1->_33 * m2->_34) + (m1->_34 * m2->_44);

	out->_41 = (m1->_41 * m2->_11) + (m1->_42 * m2->_21) + (m1->_43 * m2->_31) + (m1->_44 * m2->_41);
	out->_42 = (m1->_41 * m2->_12) + (m1->_42 * m2->_22) + (m1->_43 * m2->_32) + (m1->_44 * m2->_42);
	out->_43 = (m1->_41 * m2->_13) + (m1->_42 * m2->_23) + (m1->_43 * m2->_33) + (m1->_44 * m2->_43);
	out->_44 = (m1->_41 * m2->_14) + (m1->_42 * m2->_24) + (m1->_43 * m2->_34) + (m1->_44 * m2->_44);
}

void D3DMatrixIdentity(D3DMATRIX* mat)
{
	D3DMatrixScaling(mat, 1.0f, 1.0f, 1.0f);
}

void D3DMatrixTranslation(D3DMATRIX* mat, float x, float y, float z)
{
	D3DMatrixIdentity(mat);
	mat->_41 = x;
	mat->_42 = y;
	mat->_43 = z;
}

void D3DVec3Transform(D3DVECTOR* out, D3DVECTOR* in, D3DMATRIX* mat)
{
	out->x = (mat->_11 * in->x) + (mat->_21 * in->y) +
		(mat->_31 * in->z) + (mat->_41);
	out->y = (mat->_12 * in->x) + (mat->_22 * in->y) +
		(mat->_32 * in->z) + (mat->_42);
	out->z = (mat->_13 * in->x) + (mat->_23 * in->y) +
		(mat->_33 * in->z) + (mat->_43);
}

void D3DMatrixRotationYawPitchRoll(D3DMATRIX* mat, float yaw, float pitch, float roll)
{
	D3DMATRIX mx, my, mz, mtemp;

	D3DMatrixRotationX(&mx, pitch);
	D3DMatrixRotationY(&my, yaw);
	D3DMatrixRotationZ(&mz, roll);

	D3DMatrixMultiply(&mtemp, &mz, &mx);
	D3DMatrixMultiply(mat, &mtemp, &my);
}

void D3DMatrixInverse(D3DMATRIX* out_mat, float* out_det, D3DMATRIX* in)
{
	float inv[16], det;
	int i = 0;
	float m[16];
	m[i++] = in->_11;
	m[i++] = in->_12;
	m[i++] = in->_13;
	m[i++] = in->_14;

	m[i++] = in->_21;
	m[i++] = in->_22;
	m[i++] = in->_23;
	m[i++] = in->_24;

	m[i++] = in->_31;
	m[i++] = in->_32;
	m[i++] = in->_33;
	m[i++] = in->_34;

	m[i++] = in->_41;
	m[i++] = in->_42;
	m[i++] = in->_43;
	m[i++] = in->_44;

	inv[0] = m[5] * m[10] * m[15] -
		m[5] * m[11] * m[14] -
		m[9] * m[6] * m[15] +
		m[9] * m[7] * m[14] +
		m[13] * m[6] * m[11] -
		m[13] * m[7] * m[10];

	inv[4] = -m[4] * m[10] * m[15] +
		m[4] * m[11] * m[14] +
		m[8] * m[6] * m[15] -
		m[8] * m[7] * m[14] -
		m[12] * m[6] * m[11] +
		m[12] * m[7] * m[10];

	inv[8] = m[4] * m[9] * m[15] -
		m[4] * m[11] * m[13] -
		m[8] * m[5] * m[15] +
		m[8] * m[7] * m[13] +
		m[12] * m[5] * m[11] -
		m[12] * m[7] * m[9];

	inv[12] = -m[4] * m[9] * m[14] +
		m[4] * m[10] * m[13] +
		m[8] * m[5] * m[14] -
		m[8] * m[6] * m[13] -
		m[12] * m[5] * m[10] +
		m[12] * m[6] * m[9];

	inv[1] = -m[1] * m[10] * m[15] +
		m[1] * m[11] * m[14] +
		m[9] * m[2] * m[15] -
		m[9] * m[3] * m[14] -
		m[13] * m[2] * m[11] +
		m[13] * m[3] * m[10];

	inv[5] = m[0] * m[10] * m[15] -
		m[0] * m[11] * m[14] -
		m[8] * m[2] * m[15] +
		m[8] * m[3] * m[14] +
		m[12] * m[2] * m[11] -
		m[12] * m[3] * m[10];

	inv[9] = -m[0] * m[9] * m[15] +
		m[0] * m[11] * m[13] +
		m[8] * m[1] * m[15] -
		m[8] * m[3] * m[13] -
		m[12] * m[1] * m[11] +
		m[12] * m[3] * m[9];

	inv[13] = m[0] * m[9] * m[14] -
		m[0] * m[10] * m[13] -
		m[8] * m[1] * m[14] +
		m[8] * m[2] * m[13] +
		m[12] * m[1] * m[10] -
		m[12] * m[2] * m[9];

	inv[2] = m[1] * m[6] * m[15] -
		m[1] * m[7] * m[14] -
		m[5] * m[2] * m[15] +
		m[5] * m[3] * m[14] +
		m[13] * m[2] * m[7] -
		m[13] * m[3] * m[6];

	inv[6] = -m[0] * m[6] * m[15] +
		m[0] * m[7] * m[14] +
		m[4] * m[2] * m[15] -
		m[4] * m[3] * m[14] -
		m[12] * m[2] * m[7] +
		m[12] * m[3] * m[6];

	inv[10] = m[0] * m[5] * m[15] -
		m[0] * m[7] * m[13] -
		m[4] * m[1] * m[15] +
		m[4] * m[3] * m[13] +
		m[12] * m[1] * m[7] -
		m[12] * m[3] * m[5];

	inv[14] = -m[0] * m[5] * m[14] +
		m[0] * m[6] * m[13] +
		m[4] * m[1] * m[14] -
		m[4] * m[2] * m[13] -
		m[12] * m[1] * m[6] +
		m[12] * m[2] * m[5];

	inv[3] = -m[1] * m[6] * m[11] +
		m[1] * m[7] * m[10] +
		m[5] * m[2] * m[11] -
		m[5] * m[3] * m[10] -
		m[9] * m[2] * m[7] +
		m[9] * m[3] * m[6];

	inv[7] = m[0] * m[6] * m[11] -
		m[0] * m[7] * m[10] -
		m[4] * m[2] * m[11] +
		m[4] * m[3] * m[10] +
		m[8] * m[2] * m[7] -
		m[8] * m[3] * m[6];

	inv[11] = -m[0] * m[5] * m[11] +
		m[0] * m[7] * m[9] +
		m[4] * m[1] * m[11] -
		m[4] * m[3] * m[9] -
		m[8] * m[1] * m[7] +
		m[8] * m[3] * m[5];

	inv[15] = m[0] * m[5] * m[10] -
		m[0] * m[6] * m[9] -
		m[4] * m[1] * m[10] +
		m[4] * m[2] * m[9] +
		m[8] * m[1] * m[6] -
		m[8] * m[2] * m[5];

	det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
#if 0
	if (fabs(det) < 0.0001f)
	{
		D3DMatrixIdentity(out_mat);
		*out_det = det;
		return;
	}
#endif
	det = 1.0f / det;

	i = 0;
	out_mat->_11 = inv[i++] * det;
	out_mat->_12 = inv[i++] * det;
	out_mat->_13 = inv[i++] * det;
	out_mat->_14 = inv[i++] * det;

	out_mat->_21 = inv[i++] * det;
	out_mat->_22 = inv[i++] * det;
	out_mat->_23 = inv[i++] * det;
	out_mat->_24 = inv[i++] * det;

	out_mat->_31 = inv[i++] * det;
	out_mat->_32 = inv[i++] * det;
	out_mat->_33 = inv[i++] * det;
	out_mat->_34 = inv[i++] * det;

	out_mat->_41 = inv[i++] * det;
	out_mat->_42 = inv[i++] * det;
	out_mat->_43 = inv[i++] * det;
	out_mat->_44 = inv[i++] * det;

	*out_det = det;
}

void D3DMatrixLookAt(D3DMATRIX* out, D3DVECTOR* eye, D3DVECTOR* lookat, D3DVECTOR* up)
{
	D3DVECTOR xaxis, yaxis, zaxis;

	D3DVectorSubtract(&zaxis, eye, lookat);
	D3DVec3Normalize(&zaxis, &zaxis);

	D3DVectorCross(&xaxis, up, &zaxis);
	D3DVec3Normalize(&xaxis, &xaxis);

	D3DVectorCross(&yaxis, &zaxis, &xaxis);

	out->_11 = -xaxis.x; out->_12 = yaxis.x; out->_13 = zaxis.x; out->_14 = 0.0f;
	out->_21 = -xaxis.y; out->_22 = yaxis.y; out->_23 = zaxis.y; out->_24 = 0.0f;
	out->_31 = -xaxis.z; out->_32 = yaxis.z; out->_33 = zaxis.z; out->_34 = 0.0f;
	out->_41 = D3DVectorDot(&xaxis, eye); out->_42 = 0.0f - D3DVectorDot(&yaxis, eye); out->_43 = 0.0f - D3DVectorDot(&zaxis, eye); out->_44 = 1.0f;
}

//help function to create zbuffer
HRESULT WINAPI EnumZBufferCallback( DDPIXELFORMAT* pddpf,
                                           VOID* pddpfDesired )
{
    // For this tutorial, we are only interested in z-buffers, so ignore any
    // other formats (e.g. DDPF_STENCILBUFFER) that get enumerated. An app
    // could also check the depth of the z-buffer (16-bit, etc,) and make a
    // choice based on that, as well.
    if( pddpf->dwFlags == DDPF_ZBUFFER )
    {
        memcpy( pddpfDesired, pddpf, sizeof(DDPIXELFORMAT) );
 
        // Return with D3DENUMRET_CANCEL to end the search.
        return D3DENUMRET_CANCEL;
    }
 
    // Return with D3DENUMRET_OK to continue the search.
    return D3DENUMRET_OK;
}

#ifndef NDEBUG
int SurfRefCount = 0;
#endif

LPDIRECTDRAWSURFACE7 ZSGraphicsSystem::CreateSurface(int width, int height, int mem_flags, DWORD colour_key_value = 0)
{

	DDSURFACEDESC2 TempDescription;
	LPDIRECTDRAWSURFACE7 Surface;

	// Set appropriate values to the surface description
	memset(&TempDescription,0,sizeof(TempDescription));
	TempDescription.dwSize = sizeof(TempDescription);
	TempDescription.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_CKSRCBLT;

	// Set size
	TempDescription.dwWidth = width;
	TempDescription.dwHeight = height;

	if(!mem_flags)
	{
		// Set the type
		TempDescription.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	}
	else
	{
		TempDescription.ddsCaps.dwCaps = mem_flags;
	}
	// Create the surface
	if (FAILED(DirectDraw->CreateSurface(&TempDescription,&Surface,NULL))) {
		// That failed.
		SafeExit("Surface creation failed!");

		return NULL;
	};

	// Set the colour key to whatever was passed
	DDCOLORKEY ColourKey;
	ColourKey.dwColorSpaceLowValue = colour_key_value;
	ColourKey.dwColorSpaceHighValue = colour_key_value;

	// Set the colour key for source blitting
	Surface->SetColorKey(DDCKEY_SRCBLT,&ColourKey);

	FillSurface(Surface,colour_key_value,NULL);

	// Return the surface
	return Surface;

}

BOOL ZSGraphicsSystem::LoadFileIntoSurface(LPDIRECTDRAWSURFACE7 lpddInto, char *filename, int width, int height)
{
	HBITMAP hBM;  
	BITMAP BM;  
	HDC hDCImage, hDC;  
	
	hBM = ( HBITMAP ) LoadImage( NULL, filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION );


	if (hBM == NULL) 
	{
		hBM = ( HBITMAP ) LoadImage( NULL, filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION );		
		DEBUG_INFO(filename);
		if (hBM == NULL) 
		{
			SafeExit(filename);
		}
	};

	GetObject( hBM, sizeof( BM ), &BM );  
	
	hDCImage = CreateCompatibleDC( NULL );  
	SelectObject( hDCImage, hBM );  
	
	if( SUCCEEDED( lpddInto->GetDC( &hDC )))  
	{  
		BitBlt( hDC, 0, 0, width, height, hDCImage, 0, 0, SRCCOPY );  
		lpddInto->ReleaseDC( hDC );  
	} else 
	{
		SafeExit("getdc failed");
	};
	
	DeleteDC( hDCImage );  
	DeleteObject( hBM );

	// Return the surface
	return TRUE;
}



LPDIRECTDRAWSURFACE7 ZSGraphicsSystem::CreateSurfaceFromFile(char *filename, int width, int height, int mem_flags, DWORD colour_key_value)
{

	DDSURFACEDESC2 TempDescription;
	LPDIRECTDRAWSURFACE7 Surface;

	// Set appropriate values to the surface description
	memset(&TempDescription,0,sizeof(TempDescription));
	TempDescription.dwSize = sizeof(TempDescription);
	TempDescription.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_CKSRCBLT;

	// Set size
	TempDescription.dwWidth = width;
	TempDescription.dwHeight = height;
	//TempDescription.ddckCKSrcBlt.dwColorSpaceHighValue = RGB(255,0,255);
	//TempDescription.ddckCKSrcBlt.dwColorSpaceLowValue = RGB(255,0,255);
	TempDescription.ddckCKSrcBlt.dwColorSpaceLowValue = colour_key_value;
	TempDescription.ddckCKSrcBlt.dwColorSpaceHighValue = colour_key_value;

	if(!mem_flags)
	{
		// Set the type
		TempDescription.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	}
	else
	{
		TempDescription.ddsCaps.dwCaps = mem_flags;
	}
	// Create the surface
	if (FAILED(DirectDraw->CreateSurface(&TempDescription,&Surface,NULL))) {
		// That failed.
		SafeExit("Surface creation failed!");
		return NULL;
	};

	HBITMAP hBM;  
	BITMAP BM;  
	HDC hDCImage, hDC;  
	
	hBM = ( HBITMAP ) LoadImage( NULL, filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION );


	if (hBM == NULL) 
	{
		hBM = ( HBITMAP ) LoadImage( NULL, filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION );		
		DEBUG_INFO(filename);
		if (hBM == NULL) 
		{
			SafeExit(filename);
		}
	};

	GetObject( hBM, sizeof( BM ), &BM );  
	
	hDCImage = CreateCompatibleDC( NULL );  
	SelectObject( hDCImage, hBM );  
	
	if( SUCCEEDED( Surface->GetDC( &hDC )))  
	{  
		BitBlt( hDC, 0, 0, width, height, hDCImage, 0, 0, SRCCOPY );  
		Surface->ReleaseDC( hDC );  
	} else 
	{
		SafeExit("getdc failed");
	};
	
	DeleteDC( hDCImage );  
	DeleteObject( hBM );

	WORD Mask;
	WORD* SurfPtr;  
//	WORD AlphaMask;
//	WORD RGBMask;

	if(colour_key_value == COLOR_KEY_FROM_FILE)
	{
		ZeroMemory( &TempDescription, sizeof( TempDescription ));  
		TempDescription.dwSize = sizeof( TempDescription );  
 
		Surface->Lock( NULL, &TempDescription, DDLOCK_WAIT, NULL );  
 
		SurfPtr = ( WORD* ) (( BYTE* )TempDescription.lpSurface);

		Mask = SurfPtr[width-1];
		
		Surface->Unlock( NULL );
		
		DDCOLORKEY ddck;
		ddck.dwColorSpaceHighValue = Mask;
		ddck.dwColorSpaceLowValue = Mask;

		Surface->SetColorKey(DDCKEY_SRCBLT , &ddck);
	}
	else
	{
		// Set the colour key to whatever was passed
		DDCOLORKEY ColourKey;
		ColourKey.dwColorSpaceLowValue = colour_key_value;
		ColourKey.dwColorSpaceHighValue = colour_key_value;

		// Set the colour key for source blitting
		if(FAILED(Surface->SetColorKey(DDCKEY_SRCBLT,&ColourKey)))
		{
			SafeExit("Failed to set color key!");
			return NULL;
		}
	}
	// Return the surface
	return Surface;
}

LPDIRECTDRAWCLIPPER ZSGraphicsSystem::AttachClipper(LPDIRECTDRAWSURFACE7 surface, int RectCount, LPRECT ClipList)
{
	int Count;
	LPDIRECTDRAWCLIPPER Clipper;
	LPRGNDATA RegionData;

	// We need a clipper object to play with

	if (FAILED(DirectDraw->CreateClipper(0,&Clipper,NULL))) {
		// Clipper creation failed
		return NULL;
	};

	// Create the clip list
	RegionData = (LPRGNDATA)malloc(sizeof(RGNDATAHEADER)+RectCount*sizeof(RECT));
	memcpy(RegionData->Buffer, ClipList, sizeof(RECT)*RectCount);

	// Setup the header fields
	RegionData->rdh.dwSize          = sizeof(RGNDATAHEADER);
	RegionData->rdh.iType           = RDH_RECTANGLES;
	RegionData->rdh.nCount          = RectCount;
	RegionData->rdh.nRgnSize        = RectCount*sizeof(RECT);

	RegionData->rdh.rcBound.left    =  64000;
	RegionData->rdh.rcBound.top     =  64000;
	RegionData->rdh.rcBound.right   = -64000;
	RegionData->rdh.rcBound.bottom  = -64000;

	// Find bounds of all clipping regions
	for (Count = 0; Count < RectCount; Count++) {
		// test if the next rectangle unioned with the current bound is larger
		if (ClipList[Count].left < RegionData->rdh.rcBound.left)
			RegionData->rdh.rcBound.left = ClipList[Count].left;

		if (ClipList[Count].right > RegionData->rdh.rcBound.right)
			RegionData->rdh.rcBound.right = ClipList[Count].right;

		if (ClipList[Count].top < RegionData->rdh.rcBound.top)
			RegionData->rdh.rcBound.top = ClipList[Count].top;

		if (ClipList[Count].bottom > RegionData->rdh.rcBound.bottom)
			RegionData->rdh.rcBound.bottom = ClipList[Count].bottom;
	};

	// Set the clip list
	if (FAILED(Clipper->SetClipList(RegionData,0))) {
		// Clip list set failed
		free(RegionData);
		return(NULL);
	};

	// Attach the clipper to the surface
	if (FAILED(surface->SetClipper(Clipper))) {
		// Attachment failed
		free(RegionData);
		return(NULL);
	};

	// It worked!
	free(RegionData);
	return Clipper;
}

void ZSGraphicsSystem::FillSurface(LPDIRECTDRAWSURFACE7 lpdds, DWORD colour, RECT *prToFill)
{
	DDBLTFX BlitFX;

	// Clear the structure
	DDRAW_INIT_STRUCT(BlitFX);
	BlitFX.dwFillColor = colour;
	lpdds->Blt(prToFill, NULL,NULL, DDBLT_COLORFILL | DDBLT_WAIT,&BlitFX);

}

void ZSGraphicsSystem::DrawBox(LPDIRECTDRAWSURFACE7 Target, LPRECT rBox, DWORD Color)
{
	DDBLTFX BltFx;
	BltFx.dwSize = sizeof(BltFx);
	BltFx.dwFillColor = Color;
	RECT rDraw;
	rDraw.top = rBox->top;
	rDraw.left = rBox->left;
	rDraw.bottom = rBox->top + 1;
	rDraw.right = rBox->right + 1;
	Target->Blt(&rDraw,NULL,NULL,DDBLT_COLORFILL,&BltFx);

	rDraw.top = rBox->top;
	rDraw.left = rBox->left;
	rDraw.bottom = rBox->bottom +1;
	rDraw.right = rBox->left + 1;
	Target->Blt(&rDraw,NULL,NULL,DDBLT_COLORFILL,&BltFx);

	rDraw.top = rBox->top;
	rDraw.left = rBox->right;
	rDraw.bottom = rBox->bottom + 1;
	rDraw.right = rBox->right + 1;
	Target->Blt(&rDraw,NULL,NULL,DDBLT_COLORFILL,&BltFx);

	rDraw.top = rBox->bottom;
	rDraw.left = rBox->left;
	rDraw.bottom = rBox->bottom + 1;
	rDraw.right = rBox->right;
	Target->Blt(&rDraw,NULL,NULL,DDBLT_COLORFILL,&BltFx);
}

void ZSGraphicsSystem::Flip()
{
	// Flip pages
	if (!Windowed) 
	{
		while(FAILED(Primary->Flip(NULL,DDFLIP_WAIT)));
	} 
	else 
	{
		RECT dest;
		RECT rsource;

		// get the window's rectangle in screen coordinates
		GetWindowRect(MainWindow, &dest);
	
		rsource.top = 0;
		rsource.left = 0;
		rsource.right = ScreenWidth;
		rsource.bottom = ScreenHeight;

		if (FAILED(Primary->Blt(&dest, BBuffer, &rsource, DDBLT_WAIT,NULL))) {
			// Unable to blit, wonder why?
			SafeExit("Blit failed");
		};
	};
}

void ZSGraphicsSystem::TextureBlt(LPRECT rArea, ZSTexture *source, float x, float y, float w, float h)
{
	D3DTLVERTEX v[4];

	v[0].sx = (float)rArea->left;
	v[0].sy = (float)rArea->top;
	v[0].tu = x;
	v[0].tv = y;
	v[1].sx = (float)rArea->right;
	v[1].sy = (float)rArea->top;
	v[1].tu = x + w;
	v[1].tv = y;
	v[2].sx = (float)rArea->left;
	v[2].sy = (float)rArea->bottom;
	v[2].tu = x;
	v[2].tv = y + h;
	v[3].sx = (float)rArea->right;
	v[3].sy = (float)rArea->bottom;
	v[3].tu = x + w;
	v[3].tv = y + h;

	v[0].rhw = v[1].rhw = v[2].rhw = v[3].rhw = 1.0f;
	v[0].specular = v[1].specular = v[2].specular = v[3].specular = v[0].specular = v[1].specular = v[2].specular = v[3].specular = D3DRGB(1.0f,1.0f,1.0f);
	v[0].sz = v[1].sz = v[2].sz = v[3].sz = 0;

	D3DDevice->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);
	D3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
	//D3DDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
	//D3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	D3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);
	//D3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		         //      0x00000000, 1.0f, 0L );

	SetTexture(source);

	D3DDevice->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTFG_POINT);
	D3DDevice->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTFG_POINT);

	D3DDevice->BeginScene();

	if(FAILED(D3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,D3DFVF_TLVERTEX,v,4,0 )))
	{
		SafeExit("Texture draw failed");
	}

	D3DDevice->EndScene();
	
	
//	Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTFG_LINEAR);
//	Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTFG_LINEAR);

	//D3DDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
	//D3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	D3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE);
#if 0
	if(FAILED(Dest->Blt(rArea,BBuffer,rArea,DDBLT_WAIT,NULL)))
	{
		SafeExit("Texture blt failed");
	}
#endif
}


int ZSGraphicsSystem::CreateProjectionMatrix(float width, float height, float depth)
{
	D3DXMatrixOrtho((D3DXMATRIX*)&Projection,width,height * ((float)(ScreenHeight-20) / (float)ScreenWidth),-(depth/2.0f),depth/2.0f);

	HRESULT hr;

	hr = D3DDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION,&Projection);

	assert(hr == D3D_OK);

	return TRUE;
}

void ZSGraphicsSystem::SetViewDim(float NewDim)
{
	ViewDim = NewDim;
	if(ViewDim < MIN_ZOOM)
	{
		ViewDim = MIN_ZOOM;
	}
	else
	if(ViewDim > MAX_ZOOM)
	{
		ViewDim = MAX_ZOOM;
	}
	
	CreateProjectionMatrix(ViewDim,ViewDim, VIEW_DEPTH*4);
	return;

}


int ZSGraphicsSystem::Zoom(float offset)
{
	ViewDim += offset;
	if(ViewDim < MIN_ZOOM)
	{
		ViewDim = MIN_ZOOM;
	}
	else
	if(ViewDim > MAX_ZOOM)
	{
		ViewDim = MAX_ZOOM;
	}
	
	CreateProjectionMatrix(ViewDim,ViewDim, VIEW_DEPTH*4);
	return TRUE;
}

BOOL ZSGraphicsSystem::SetTexture(ZSTexture *ToSet)
{
	if(pCurTexture != ToSet)
	{
		if(ToSet)
		{
			if(FAILED(D3DDevice->SetTexture(0,ToSet->GetSurface())))
			{
				DEBUG_INFO("Could not set Texture.\n");
				return FALSE;
			}
			else
			{
				pCurTexture = ToSet;
			}
		}
		else
		{
			if(FAILED(D3DDevice->SetTexture(0,0)))
			{
				DEBUG_INFO("Could not clear Texture.\n");
				return FALSE;
			}
			else
			{
				pCurTexture = NULL;
				return TRUE;	
			}

		}
	}
	return TRUE;
}

BOOL ZSGraphicsSystem::ClearTexture()
{
	if(FAILED(D3DDevice->SetTexture(0,0)))
	{
		DEBUG_INFO("Could not clear Texture.\n");
		return FALSE;
	}
	else
	{
		pCurTexture = NULL;
		return TRUE;	
	}
}

#ifdef USE_SDL
HWND ZSGraphicsSystem::Init()
#else
HWND ZSGraphicsSystem::Init(HINSTANCE hInstance)
#endif
{
	DEBUG_INFO("Beginning Graphics system Init\n");

	FILE *fp;
	int n;

	DDSURFACEDESC2	ddsd;
#ifdef USE_SDL
	SDL_SysWMinfo info;
	int window_flags = 0;
#else
	WNDCLASS winclass;	// this will hold the class we create
	// first fill in the window class stucture
	
	winclass.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;// | OWNDC
	winclass.lpfnWndProc	= DefWindowProc;
	winclass.cbClsExtra		= 0;
	winclass.cbWndExtra		= 0;
	winclass.hInstance		= hInstance;
	winclass.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	winclass.hCursor		= LoadCursor(NULL, IDC_ARROW);
	winclass.hbrBackground	= (HBRUSH)GetStockObject(GRAY_BRUSH);
	winclass.lpszMenuName	= NULL;
	winclass.lpszClassName	= WINDOW_CLASS_NAME;

	Application = hInstance;

	// register the window class
	if (!RegisterClass(&winclass))
		return(0);
#endif
	DEBUG_INFO("Getting screen information from GUI.INI, ");

	fp = SafeFileOpen("gui.ini","rt");

	SeekTo(fp,"[MAIN]");

	SeekTo(fp,"WIDTH");

	ScreenWidth = GetInt(fp);
	
	SeekTo(fp, "HEIGHT");

	ScreenHeight = GetInt(fp);
	
	SeekTo(fp, "WINDOWED");

	Windowed = GetInt(fp);

	fclose(fp);

	DEBUG_INFO(" done.\n\n")
	
	DEBUG_INFO("Creating Windows interface Window\n");

#ifndef USE_SDL
	MainWindow = CreateWindowEx(0,
			WINDOW_CLASS_NAME,
			WINDOW_TITLE,
			WS_POPUP,
			0,
			0,
			ScreenWidth,
         ScreenHeight,
			NULL,
			NULL,
			hInstance,
			NULL );
#else
	if (SDL_Init(SDL_INIT_EVERYTHING))
	{
		return NULL;
	}

	window = SDL_CreateWindow("Prelude to Darkness", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, ScreenWidth, ScreenHeight, window_flags);

	if (!window)
	{
		return NULL;
	}

	SDL_VERSION(&info.version);
	SDL_GetWindowWMInfo(window, &info);

	MainWindow = info.info.win.window;

#endif
	DEBUG_INFO("Windows interface window created\n\n");

	ShowWindow(MainWindow, SW_SHOW);
	
	DEBUG_INFO("Initializing D3DX\n");
	D3DXInitialize();
	DEBUG_INFO("D3DX init done\n");
	

	BitDepth = 16;

	//Init Direct Draw
	// Set some variables to NULL for the sake of safety
	Clipper = NULL;
	WindowClipper = NULL;
	DirectDraw = NULL;
	Primary = NULL;
	BBuffer = NULL;
	ViewDim = 11.5f;

	// Temporary variable used to get DirectDraw 6
	LPDIRECTDRAW lpddTemp = NULL;

	DEBUG_INFO("Getting DirectDraw 1\n");
	// Get DirectDraw 1
	if (FAILED(DirectDrawCreateEx( NULL, (VOID**)&lpddTemp, IID_IDirectDraw7, NULL ))) {
		// Unable to init basic DirectDraw
		SafeExit("Unable to init basic DirectDraw.");
	};

	DEBUG_INFO("upgrading to DirectDraw 7\n");
	// Get latest DirectDraw 
	if (FAILED(lpddTemp->QueryInterface(IID_IDirectDraw7,(LPVOID *)&DirectDraw))) {
		SafeExit("Unable to upgrade to DirectDraw 7.0.");
	};

	// Set Cooperation level, based on Window/Fullscreen
	if (Windowed) 
	{
		DEBUG_INFO("Setting Cooperation level for Windowed App, ");
		// Windowed apps like DDSCL_NORMAL
		if (FAILED(DirectDraw->SetCooperativeLevel(MainWindow,DDSCL_NORMAL))) 
		{
			SafeExit("Setting Cooperation failed");
		};
		DEBUG_INFO("done\n");
	
	} 
	else 
	{
		DEBUG_INFO("Setting Cooperation level for FullScreen App, ");
		// Fullscreen apps (16bpp) should have more
		if (FAILED(DirectDraw->SetCooperativeLevel(MainWindow,DDSCL_ALLOWMODEX | DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE | DDSCL_ALLOWREBOOT | DDSCL_FPUSETUP))) 
		{
			SafeExit("Setting Cooperation failed");
		};
		DEBUG_INFO("done\n");
	};

// Create the desired screen mode. 16 bpp fails to 24 when necessary.
	if (!Windowed) 
	{ //changing screen mode when windowed is retarted!
	
		DEBUG_INFO("Setting Display mode to 16 bpp, ");
		if (FAILED(DirectDraw->SetDisplayMode(ScreenWidth,ScreenHeight,BitDepth,0,0))) {
			// Screen mode failed
			
			if (BitDepth == 16) 
			{
				DEBUG_INFO("failed.  Setting display mode to 24 bpp, ");

				BitDepth = 24;
				if (FAILED(DirectDraw->SetDisplayMode(ScreenWidth,ScreenHeight,BitDepth,0,0))) 
				{
					SafeExit("No 24bpp either.  Hardware incompatible.");
				};
			} 
			else 
			{
				SafeExit("FAILED set bit depth.  Hardware incompatible.");
			};
		};
		DEBUG_INFO("done.\n");
	};

	// Blank the Surface Description
	memset(&ddsd,0,sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	// If its windowed, we don't want a complex Primary surface - otherwise,
	// we do.

	if (!Windowed) 
	{
		// Fullscreen mode
		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE  | DDSCAPS_COMPLEX | DDSCAPS_FLIP | DDSCAPS_3DDEVICE;
   
		// set the backbuffer count to 0 for windowed mode
		// 1 for fullscreen mode, 2 for triple buffering
		ddsd.dwBackBufferCount = 1;
	} 
	else 
	{
		// Windowed mode
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE;
	   // set the backbuffer count to 0 for windowed mode
		// 1 for fullscreen mode, 2 for triple buffering
		ddsd.dwBackBufferCount = 0;
	};


	
	DEBUG_INFO("Create the Primary Surface, ");
	if(FAILED(DirectDraw->CreateSurface(&ddsd,&Primary,NULL)))
	{
		SafeExit("Problem creating the Primary Surface.");
	}
	DEBUG_INFO("done\n");

	// Get the pixel format (555/565)
	DDPIXELFORMAT ddpf;
	DDRAW_INIT_STRUCT(ddpf);
	Primary->GetPixelFormat(&ddpf);
	PixelFormat = ddpf.dwRGBBitCount;

	// Get a backbuffer if its a full screen mode
	if (!Windowed) 
	{
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER | DDSCAPS_3DDEVICE;
		ddsd.dwBackBufferCount = 1;
 
		DEBUG_INFO("Getting fullscreen back buffer, ");
		if (FAILED(Primary->GetAttachedSurface(&ddsd.ddsCaps,&BBuffer))) 
		{
			// No back buffer is possible
			SafeExit("failed attach back buffer.");
		};
		DEBUG_INFO("done.\n");
	} 
	else 
	{
		DEBUG_INFO("Getting windowed offscreen back buffer, ");
		BBuffer = CreateSurface(GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN),DDSCAPS_3DDEVICE,0);
		if(!BBuffer)
		{
			SafeExit("failed attach windowed bb.");
		}
		DEBUG_INFO("done.\n");
	};

	// Clear primary and secondary surfaces
	
	if (Windowed) 
	{
		DEBUG_INFO("Clearing back buffer\n");
		FillSurface(BBuffer,0,NULL);
	} 
	else 
	{
		DEBUG_INFO("Clearing Back buffer\n");
		FillSurface(BBuffer,0,NULL);
		DEBUG_INFO("Clearing Primary\n");
		FillSurface(Primary,0,NULL);
	};

	// Set software clipping region
	ClipMinX = 0;
	ClipMaxX = ScreenWidth -1;
	ClipMinY = 0;
	ClipMaxY = ScreenHeight -1;

	// Setup a clipper for the backbuffer/window
	RECT ScreenRect = {0,0,ScreenWidth,ScreenHeight};

	DEBUG_INFO("Attaching Clipper to back buffer\n");
	Clipper = AttachClipper(BBuffer,1,&ScreenRect);

	if (Windowed) 
	{
		if (FAILED(DirectDraw->CreateClipper(0,&WindowClipper,NULL))) {
			// Clipper creation failed
			SafeExit("Failed to create a clipper");
		};
		if (FAILED(WindowClipper->SetHWnd(0, MainWindow))) {
			// Clipper creation failed
			SafeExit("Failed to attach clipper to window");
		};

		if (FAILED(Primary->SetClipper(WindowClipper))) {
			// Clipper creation failed
			SafeExit("Failed to set the clipper on the primary buffer");
		};
   };


	//init Direct3D
	if (FAILED(DirectDraw->QueryInterface(IID_IDirect3D7, (LPVOID *)&Direct3D))) {
		// Unable to find Direct3D
		SafeExit("Direct3D Query Failed.  Hardware incompatible.");
	};

	// Create the ZBuffer
	ZeroMemory(&ddsd,sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags        = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER;
    ddsd.dwWidth        = GetSystemMetrics(SM_CXSCREEN);//ScreenWidth;
    ddsd.dwHeight       = GetSystemMetrics(SM_CYSCREEN);

   // Get the pixel format (555/565)

	DDRAW_INIT_STRUCT(ddpf); 

	BBuffer->GetPixelFormat(&ddpf);

	Direct3D->EnumZBufferFormats( IID_IDirect3DHALDevice, EnumZBufferCallback,  &ddpf);
	
	memcpy( &ddsd.ddpfPixelFormat, &ddpf, sizeof(DDPIXELFORMAT) );
 
    
    // For hardware devices, the z-buffer should be in video memory. For
    // software devices, create the z-buffer in system memory
    ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
    
    // Create and attach a z-buffer
    if( FAILED(DirectDraw->CreateSurface( &ddsd, &ZBuf, NULL ) ) )
	{
		SafeExit("unable to create zbuf.  Videocard or driver incompatible.");
	}

	if( FAILED(BBuffer->AddAttachedSurface( ZBuf ) ) )
	{
		SafeExit("unable to attach zbuf.  Videocard or driver incompatible.");
	}
      
	
	// Create a device
	if (FAILED(Direct3D->CreateDevice(IID_IDirect3DHALDevice,BBuffer,&D3DDevice))) {
		DEBUG_INFO("Failed to create a HAL device, trying MMX.\n");
		
		if (FAILED(Direct3D->CreateDevice(IID_IDirect3DMMXDevice,BBuffer,&D3DDevice))) {
			DEBUG_INFO("Failed to create an MMX device, trying RGB\n");
		
			if (FAILED(Direct3D->CreateDevice(IID_IDirect3DRGBDevice,BBuffer,&D3DDevice))) {
				DEBUG_INFO("Failed to create an RGB device, trying RAMP.\n");
			
				if (FAILED(Direct3D->CreateDevice(IID_IDirect3DRampDevice,BBuffer,&D3DDevice))) {
					SafeExit("No Direct3D Devices are available.  Videocard or driver incompatible.");
				};
			};

		};
	}
	else
	{
		D3DHardware = TRUE;
	};

	D3DDEVICEDESC7 D3DDevDesc;
	ZeroMemory(&D3DDevDesc, sizeof(D3DDEVICEDESC7));
	D3DDevice->GetCaps(&D3DDevDesc);



	// Create the viewport
    DWORD dwRenderWidth  = ScreenWidth;
    DWORD dwRenderHeight = ScreenHeight - 100;
    D3DVIEWPORT7 vp = { 0, 0, dwRenderWidth, dwRenderHeight, 0.0f, 1.0f };

    if(FAILED(D3DDevice->SetViewport( &vp )))
	 {
		SafeExit("Failed to set Viewport data");
	};

	// Enable z-buffering
    D3DDevice->SetRenderState( D3DRENDERSTATE_ZENABLE, TRUE );

	 DEBUG_INFO("Setting up basic colorred materials\n");
	//setup the default material
	D3DCOLORVALUE full;
	full.r = 1.0f;
	full.g = 1.0f;
	full.b = 1.0f;
	full.a = 1.0f; 

	D3DCOLORVALUE none;
	none.r = 0.0f;
	none.g = 0.0f;
	none.b = 0.0f;
	none.a = 0.5f;

	Materials[COLOR_NONE].diffuse = full; 
	Materials[COLOR_NONE].diffuse.a = 0.4f;
	Materials[COLOR_NONE].specular = full;
	Materials[COLOR_NONE].emissive = none;
	Materials[COLOR_NONE].ambient = full;
	Materials[COLOR_NONE].ambient.a = 0.4f;
	Materials[COLOR_NONE].power = 1.0;

	Materials[COLOR_RED] = Materials[COLOR_NONE];
	Materials[COLOR_RED].ambient.r = Materials[COLOR_RED].diffuse.r = 1.0f;
	Materials[COLOR_RED].ambient.g = Materials[COLOR_RED].diffuse.g = 0.05f;
	Materials[COLOR_RED].ambient.b = Materials[COLOR_RED].diffuse.b = 0.05f;
	Materials[COLOR_RED].ambient.a = Materials[COLOR_RED].diffuse.a = 0.5f;

	Materials[COLOR_ORANGE] = Materials[COLOR_NONE];
	Materials[COLOR_ORANGE].ambient.r = Materials[COLOR_ORANGE].diffuse.r = 0.66f;
	Materials[COLOR_ORANGE].ambient.g = Materials[COLOR_ORANGE].diffuse.g = 0.33f;
	Materials[COLOR_ORANGE].ambient.b = Materials[COLOR_ORANGE].diffuse.b = 0.05f;
	Materials[COLOR_ORANGE].ambient.a = Materials[COLOR_ORANGE].diffuse.a = 0.5f;


	Materials[COLOR_GREEN] = Materials[COLOR_NONE];
	Materials[COLOR_GREEN].ambient.r = Materials[COLOR_GREEN].diffuse.r = 0.05f;
	Materials[COLOR_GREEN].ambient.g = Materials[COLOR_GREEN].diffuse.g = 1.0f;
	Materials[COLOR_GREEN].ambient.b = Materials[COLOR_GREEN].diffuse.b = 0.05f;
	Materials[COLOR_GREEN].ambient.a = Materials[COLOR_GREEN].diffuse.a = 0.25f;

	Materials[COLOR_BLUE] = Materials[COLOR_NONE];
	Materials[COLOR_BLUE].ambient.r = Materials[COLOR_BLUE].diffuse.r = 0.05f;
	Materials[COLOR_BLUE].ambient.g = Materials[COLOR_BLUE].diffuse.g = 0.05f;
	Materials[COLOR_BLUE].ambient.b = Materials[COLOR_BLUE].diffuse.b = 1.0f;
	Materials[COLOR_BLUE].ambient.a = Materials[COLOR_BLUE].diffuse.a = 0.25f;

	Materials[COLOR_YELLOW] = Materials[COLOR_NONE];
	Materials[COLOR_YELLOW].ambient.r = Materials[COLOR_YELLOW].diffuse.r = 0.5f;
	Materials[COLOR_YELLOW].ambient.g = Materials[COLOR_YELLOW].diffuse.g = 0.5f;
	Materials[COLOR_YELLOW].ambient.b = Materials[COLOR_YELLOW].diffuse.b = 0.05f;
	Materials[COLOR_YELLOW].ambient.a = Materials[COLOR_YELLOW].diffuse.a = 0.25f;

	Materials[COLOR_PURPLE] = Materials[COLOR_NONE];
	Materials[COLOR_PURPLE].ambient.r = Materials[COLOR_PURPLE].diffuse.r = 0.5f;
	Materials[COLOR_PURPLE].ambient.g = Materials[COLOR_PURPLE].diffuse.g = 0.05f;
	Materials[COLOR_PURPLE].ambient.b = Materials[COLOR_PURPLE].diffuse.b = 0.5f;
	Materials[COLOR_PURPLE].ambient.a = Materials[COLOR_PURPLE].diffuse.a = 0.25f;

	Materials[COLOR_WHITE] = Materials[COLOR_NONE];
	Materials[COLOR_WHITE].ambient.r = Materials[COLOR_WHITE].diffuse.r = 1.0f;
	Materials[COLOR_WHITE].ambient.g = Materials[COLOR_WHITE].diffuse.g = 1.0f;
	Materials[COLOR_WHITE].ambient.b = Materials[COLOR_WHITE].diffuse.b = 1.0f;
	Materials[COLOR_WHITE].ambient.a = Materials[COLOR_WHITE].diffuse.a = 0.25f;

	Materials[COLOR_BLACK] = Materials[COLOR_NONE];
	Materials[COLOR_BLACK].ambient.r = Materials[COLOR_BLACK].diffuse.r = 0.0f;
	Materials[COLOR_BLACK].ambient.g = Materials[COLOR_BLACK].diffuse.g = 0.0f;
	Materials[COLOR_BLACK].ambient.b = Materials[COLOR_BLACK].diffuse.b = 0.0f;
	Materials[COLOR_BLACK].ambient.a = Materials[COLOR_BLACK].diffuse.a = 0.5f;

	Materials[COLOR_DEFAULT].emissive = none;
	DEBUG_INFO("done with basic materials.\n");

	D3DDevice->SetMaterial(&Materials[COLOR_NONE]);

	//setup the identity matrix
	Identity._11 = Identity._22 = Identity._33 = Identity._44 = 1.0f;
   Identity._12 = Identity._13 = Identity._14 = Identity._41 = 0.0f;
   Identity._21 = Identity._23 = Identity._24 = Identity._42 = 0.0f;
   Identity._31 = Identity._32 = Identity._34 = Identity._43 = 0.0f;

	DEBUG_INFO("set up the keymask.\n");
	ZSTexture temp;

	KeyMask = 0;

	temp.Load("keymask.bmp",D3DDevice,DirectDraw,&KeyMask,16,16);
	
	CreateProjectionMatrix(ViewDim, ViewDim, 80.0);

	DEBUG_INFO("set up the drawing rectangles from the mouse cursor\n");
	for(n = 0; n < (NUM_CURSORS * MAX_CURSOR_FRAMES); n++)
	{
		MouseCursor[n].left = (n % 5) * MOUSE_CURSOR_WIDTH;
		MouseCursor[n].top = (n / 5) * MOUSE_CURSOR_HEIGHT;
		MouseCursor[n].bottom = MouseCursor[n].top + MOUSE_CURSOR_HEIGHT;
		MouseCursor[n].right = MouseCursor[n].left + MOUSE_CURSOR_WIDTH;
	}

	DEBUG_INFO("Load relevant cursor data from GUI.ini\n");

	fp = SafeFileOpen("gui.ini","rt");

	SeekTo(fp,"CURSORS");

	for(n = 0; n < NUM_CURSORS; n++)
	{
		SeekTo(fp, "FRAMES:");
		NumCursorFrames[n] = GetInt(fp);

		SeekTo(fp, "X:");	
		CursorOffsetX[n] = GetInt(fp);

		SeekTo(fp, "Y:");
		CursorOffsetY[n] = GetInt(fp);
	}

	fclose(fp);

	CurMouseCursor = 0;
	Cursor = 0;
	CursorFrame = 0;
	CursorSubFrame = 0;
	CursorFrameRate = 3;
	

	DEBUG_INFO("Loading the mourse cursor file, ");
#ifndef NO_DDRAW
	MouseSurface = CreateSurfaceFromFile("mousecursor.bmp", 160, 384, NULL, COLOR_KEY_FROM_FILE);
#else
	MouseTexture = new ZSTexture("mousecursor.bmp", D3DDevice, DirectDraw, &KeyMask, 160, 384);
#endif

	DEBUG_INFO("done.\n\n");

	DEBUG_INFO("Creating ZSFontEngine.\n");
	pZSFont = new ZSFontEngine(DirectDraw);
	assert(pZSFont);

	DEBUG_INFO("ZSFontEngine created.\n");

	DEBUG_INFO("Graphics engine Init done.\n\n");

	
	FilterState = FILTER_BOTH;

	CircleFrame = 0;

	return MainWindow;

}

ZSGraphicsSystem::ZSGraphicsSystem()
{
		D3DDevice = NULL;

		Direct3D = NULL;
		Clipper = NULL;
		WindowClipper = NULL;
#ifndef NO_DDRAW
		MouseSurface = NULL;
#else
		MouseTexture = NULL;
#endif
		ZBuf = NULL;
		BBuffer = NULL;
		Primary = NULL;
		DirectDraw = NULL;
		pZSFont = NULL;

		MainWindow = NULL;

	return;
}

int ZSGraphicsSystem::ShutDown()
{
	// Shutdown Direct3D
	if (D3DDevice) 
	{
		D3DDevice->Release();
		D3DDevice = NULL;
	}

	if (Direct3D) {
		Direct3D->Release();
		Direct3D = NULL;
	}

	// Shutdown DirectDraw
	if (Clipper) 
	{
		Clipper->Release();
		Clipper = NULL;
	}

	if (WindowClipper) 
	{
		WindowClipper->Release();
		WindowClipper = NULL;
	}

	//release the surfaces
#ifndef NO_DDRAW
	if (MouseSurface)
	{
		MouseSurface->Release();
		MouseSurface = NULL;
	}
#else
	delete MouseTexture;
#endif
	if (ZBuf) 
	{
		ZBuf->Release();
		ZBuf = NULL;
	}

	if (BBuffer) 
	{
		BBuffer->Release();
		BBuffer = NULL;
	}

	if (Primary) 
	{
		Primary->Release();
		Primary = NULL;
	}

	if (DirectDraw) 
	{
		DirectDraw->Release();
		DirectDraw = NULL;
	}

	if(pZSFont)
	{
		delete pZSFont;
		pZSFont = NULL;
	}
#ifdef USE_SDL
	SDL_DestroyWindow(window);
	SDL_Quit();
#else
	if(MainWindow)
	DestroyWindow(MainWindow);
#endif
	return TRUE;
}


void ZSGraphicsSystem::AdvanceCursorFrame()
{
	//only the base hand cursor does not animate
	CursorSubFrame++;
	if(CursorSubFrame == CursorFrameRate)
	{
		CursorSubFrame = 0;
		AdvanceCircleFrame();
		if(Cursor == 0)
		{

		}
		else
		{
			CursorFrame ++;
			if(CursorFrame == NumCursorFrames[Cursor])
			{
				CursorFrame = 0;
			}
		}
	}
	
	
}

void ZSGraphicsSystem::SetCursorFrame(int FrameNum)
{
	CursorFrame = FrameNum;
}

void ZSGraphicsSystem::SetCursor(int CursorNum)
{
	Cursor = CursorNum;
	if(CursorFrame >= NumCursorFrames[Cursor])
	{
		CursorFrame = 0;
	}
	if(CursorNum == 0)
	{
		CursorFrame = 0;
	}
}

void ZSGraphicsSystem::DrawCursor(RECT *rDrawAt)
{
	RECT rDrawTo = *rDrawAt;
	RECT* cursor_rect = &MouseCursor[(Cursor * MAX_CURSOR_FRAMES) + CursorFrame];
	float x, y, w, h;
	rDrawTo.left -= CursorOffsetX[Cursor];
	rDrawTo.top -= CursorOffsetY[Cursor];
	rDrawTo.right -= CursorOffsetX[Cursor];
	rDrawTo.bottom -= CursorOffsetY[Cursor];
#ifndef NO_DDRAW
	BBuffer->Blt(&rDrawTo, MouseSurface, cursor_rect, DDBLT_KEYSRC, NULL);
#else
	w = 32.0f / 160.0f;
	h = 32.0f / 384.0f;
	x = (float)cursor_rect->left / 160.0f;
	y = (float)cursor_rect->top / 384.0f;
	TextureBlt(&rDrawTo, MouseTexture, x, y, w, h);
#endif
}

HRESULT ZSGraphicsSystem::DrawText(int x, int y, char *Text)
{
	return pZSFont->DrawText(BBuffer, x, y, Text);
}

HRESULT ZSGraphicsSystem::DrawText(D3DVECTOR *pVector, char *Text)
{
	int x;
	int y;

	D3DXMATRIX matView;
	D3DXMATRIX matProj;

	Engine->Graphics()->GetD3D()->GetTransform(D3DTRANSFORMSTATE_VIEW, (D3DMATRIX *)&matView);
	Engine->Graphics()->GetD3D()->GetTransform(D3DTRANSFORMSTATE_PROJECTION, (D3DMATRIX *)&matProj);

	D3DXVECTOR4 vScreen;
	
	D3DXMATRIX matFinal;

	D3DXMatrixMultiply(&matFinal,&matView,&matProj);
	
	D3DXVec3Transform(&vScreen,(D3DXVECTOR3 *)pVector,&matFinal);
	
	float WidthFactor;
	float HeightFactor;
	WidthFactor = (float)ScreenWidth / 2.0f;
	HeightFactor = (float)(ScreenHeight - 100) / 2.0f;


	x = (int)((vScreen.x * WidthFactor) + WidthFactor);
	y = (int)((-vScreen.y * HeightFactor) + HeightFactor);

	int TextWidth;
	TextWidth = pZSFont->GetTextWidth(Text);

	return pZSFont->DrawText(BBuffer, x - (TextWidth / 2), y - pZSFont->GetTextHeight()/2, Text);

}
	

HRESULT ZSGraphicsSystem::DrawText(RECT *rArea, char *Text)
{
	return pZSFont->DrawText(BBuffer, rArea, Text);
}

void ZSGraphicsSystem::SetFont(ZSFont *pFont)
{
	pZSFont->SelectDisplayFont(pFont);
}

void ZSGraphicsSystem::OutPutDebugInfo(FILE *fp)
{



}
void ZSGraphicsSystem::SetUpCircles()
{
	CircleTextures[0] = Engine->GetTexture("circle1");
	CircleTextures[1] = Engine->GetTexture("circle2");
	CircleTextures[2] = Engine->GetTexture("circle3");
}

int ZSGraphicsSystem::Circle(D3DVECTOR *vAt, COLOR_T Color, float Radius)
{

	SetTexture(CircleTextures[CircleFrame]);
	D3DLVERTEX CircVerts[4];

	CircVerts[0].z = CircVerts[1].z = CircVerts[2].z = CircVerts[3].z = vAt->z;
	CircVerts[0].x = CircVerts[2].x = vAt->x - Radius;
	CircVerts[1].x = CircVerts[3].x = vAt->x + Radius;
	CircVerts[0].y = CircVerts[1].y = vAt->y - Radius;
	CircVerts[2].y = CircVerts[3].y = vAt->y + Radius;

	CircVerts[0].tu = 0.0f;
	CircVerts[0].tv = 0.0f;
	CircVerts[1].tu = 1.0f;
	CircVerts[1].tv = 0.0f;
	CircVerts[2].tu = 0.0f;
	CircVerts[2].tv = 1.0f;
	CircVerts[3].tu = 1.0f;
	CircVerts[3].tv = 1.0f;

	CircVerts[0].color = CircVerts[1].color = CircVerts[2].color = CircVerts[3].color = D3DRGBA(1.0f,1.0f,1.0f,1.0f);
	CircVerts[0].specular = CircVerts[1].specular = CircVerts[2].specular = CircVerts[3].specular = D3DRGB(1.0f,1.0f,1.0f);

	D3DDevice->SetMaterial(&Materials[Color]);

	HRESULT hr;
	hr = D3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_LVERTEX, CircVerts, 4, 0 );

	if(hr != D3D_OK)
	{
		SafeExit("failed to draw cirlce");
	}

	return TRUE;
}

void ZSGraphicsSystem::SetRenderState(D3DRENDERSTATETYPE dwType, DWORD dwState)
{
	if(FAILED(D3DDevice->SetRenderState(dwType,dwState)))
	{
		SafeExit("Failed to Set Render State:\n");
	}
}