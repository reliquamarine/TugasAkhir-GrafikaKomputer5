/*
 * ==============================================================================
 * PROYEK AKHIR PRAKTIKUM GRAFIKA KOMPUTER
 * JUDUL: Abyss Explorer: Deep Sea Adventure
 * 
 * TIM PENGEMBANG:
 * 1. Frito Radestya S.        (241401030) - Pemodelan 3D & Pencahayaan (Kapal, Peti, Spotlight)
 * 2. Muhammad Al Farel Azhar (241401009) - Navigasi & Biota Laut (Ikan, Karang, Batu, Gelembung)
 * 
 * S-1 Ilmu Komputer - Universitas Sumatera Utara
 * Dosen Pengampu: Herriance, S.T., M.Kom.
 * ==============================================================================
 */

#define _CRT_SECURE_NO_WARNINGS
#include <glut.h>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <cstring>

// Definisi Konstanta Matematika & Batas Dunia
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

const float DEG2RAD = M_PI / 180.0f;
const float RAD2DEG = 180.0f / M_PI;

// Batas Palung Samudra
const float WORLD_LIMIT_X = 45.0f;
const float WORLD_LIMIT_Z = 45.0f;
const float OCEAN_CEILING = 10.0f;
const float OCEAN_FLOOR   = -24.0f;

// ==============================================================================
// STRUKTUR DATA & STATE GLOBAL
// ==============================================================================

// Kapal Selam (Submarine) State
struct Submarine {
    float x, y, z;          // Posisi di laut dalam
    float yaw;              // Rotasi hadap (kiri/kanan)
    float pitch;            // Rotasi menukik / mendongak
    float roll;             // Kemiringan lateral
    float speed;            // Kecepatan gerak
    float propAngle;        // Rotasi sudut baling-baling
    float scale;            // Skala lokal kapal (PageUp / PageDown)
    bool  spotlightOn;      // Status lampu sorot (F1)
    bool  isBoosting;       // Status tombol Space
} sub;

// Partikel Gelembung 3D
struct Bubble {
    float x, y, z;
    float vx, vy, vz;
    float size;
    float maxScale;
    float wobblePhase;
    float rotAngle;         // Sudut rotasi 3D partikel (glRotatef)
    float rotSpeed;         // Kecepatan rotasi partikel
    float rotAxisX;         // Komponen sumbu rotasi X
    float rotAxisY;         // Komponen sumbu rotasi Y
    float rotAxisZ;         // Komponen sumbu rotasi Z
    bool  active;
};
const int MAX_BUBBLES = 120;
Bubble bubbles[MAX_BUBBLES];

// Biota Laut: Ikan
struct Fish {
    float centerX, centerY, centerZ;
    float radiusX, radiusZ;
    float currentAngle;
    float speed;
    float vertAmplitude;
    float vertSpeed;
    float tailWagPhase;
    float colorR, colorG, colorB;
    float sizeScale;
    float posX, posY, posZ;
    float yaw;
};
const int NUM_FISH = 6;
Fish schoolOfFish[NUM_FISH];

// Biota Laut: Terumbu Karang (8 Titik Koordinat Sesuai Rencana Proposal)
struct Coral {
    float x, y, z;
    float heightScale;
    float rotY;
    float colorR, colorG, colorB;
};
const int NUM_CORALS = 8;
Coral corals[NUM_CORALS] = {
    { -22.0f, OCEAN_FLOOR, -18.0f, 1.3f,   0.0f, 0.95f, 0.35f, 0.45f }, // Coral Merah Muda
    {  18.0f, OCEAN_FLOOR, -22.0f, 1.1f,  45.0f, 0.20f, 0.85f, 0.70f }, // Coral Cyan
    { -15.0f, OCEAN_FLOOR,  20.0f, 1.4f,  90.0f, 0.95f, 0.60f, 0.15f }, // Coral Jingga Emas
    {  25.0f, OCEAN_FLOOR,  15.0f, 0.9f, 135.0f, 0.75f, 0.25f, 0.90f }, // Coral Violet
    {  -5.0f, OCEAN_FLOOR, -30.0f, 1.5f, 180.0f, 0.25f, 0.90f, 0.40f }, // Coral Hijau Neon
    {  10.0f, OCEAN_FLOOR,  32.0f, 1.2f, 225.0f, 0.95f, 0.40f, 0.60f }, // Coral Magenta
    { -30.0f, OCEAN_FLOOR,   5.0f, 1.0f, 270.0f, 0.85f, 0.80f, 0.20f }, // Coral Kuning Terang
    {  32.0f, OCEAN_FLOOR, -10.0f, 1.3f, 315.0f, 0.30f, 0.65f, 0.95f }  // Coral Biru Laut
};

// Formasi Batu Dasar Laut (Keliling Tepi Palung Laut)
struct SeaRock {
    float x, y, z;
    float rotX, rotY, rotZ;
    float scaleX, scaleY, scaleZ;
};
const int NUM_ROCKS = 18;
SeaRock rocks[NUM_ROCKS];

// Peti Harta Karun (Treasure Chest)
struct TreasureChest {
    float x, y, z;
    float spinAngle;
    float pulseTime;
    bool  collected;
} chest;

// Status & Pengaturan Kamera
enum CameraMode { CAM_CHASE, CAM_ORBIT };
CameraMode currentCamMode = CAM_CHASE;
float orbitYaw   = 45.0f;
float orbitPitch = 25.0f;
float orbitDist  = 18.0f;

// Pengaturan Skala Global & Biota
float globalScale     = 1.0f;  // Kontrol tombol + dan -
float marineLifeScale = 1.0f;  // Kontrol Home dan End
float envRotation     = 0.0f;  // Kontrol Q / E untuk rotasi lingkungan

// Game Mechanics & Telemetri
int   treasuresCollected = 0;
bool  fishCollisionAlert = false;
int   fishCollisionCount = 0;
float alertTimer = 0.0f;
float collisionPenaltyTimer = 0.0f;   // Durasi aktif penalti kecepatan (~1.0s)
float collisionCooldownTimer = 0.0f;  // Cooldown agar benturan tidak spam setiap frame (~1.6s)
bool  isPenalized = false;            // Status penalti benturan aktif
float globalAnimTime = 0.0f;

// Window Properties
int windowWidth  = 1280;
int windowHeight = 720;

// ==============================================================================
// PROSEDUR INISIALISASI GAMEPLAY & GRAFIKA
// ==============================================================================

void relocateTreasure() {
    // Menempatkan peti di koordinat acak dasar laut dalam batas palung
    float angle = (rand() % 360) * DEG2RAD;
    float radius = 10.0f + (rand() % 25);
    chest.x = radius * cos(angle);
    chest.z = radius * sin(angle);
    chest.y = OCEAN_FLOOR + 1.0f;
    chest.spinAngle = 0.0f;
    chest.pulseTime = 0.0f;
    chest.collected = false;
}

void initGame() {
    // Reset Kapal Selam
    sub.x = 0.0f;
    sub.y = -10.0f;
    sub.z = 10.0f;
    sub.yaw = 0.0f;
    sub.pitch = 0.0f;
    sub.roll = 0.0f;
    sub.speed = 0.0f;
    sub.propAngle = 0.0f;
    sub.scale = 1.0f;
    sub.spotlightOn = true;
    sub.isBoosting = false;

    // Reset Skala
    globalScale = 1.0f;
    marineLifeScale = 1.0f;
    envRotation = 0.0f;

    // Reset Skor & Kamera
    treasuresCollected = 0;
    fishCollisionCount = 0;
    fishCollisionAlert = false;
    isPenalized = false;
    collisionPenaltyTimer = 0.0f;
    collisionCooldownTimer = 0.0f;
    currentCamMode = CAM_CHASE;
    orbitYaw = 45.0f;
    orbitPitch = 25.0f;

    // Inisialisasi Peti Harta
    relocateTreasure();

    // Inisialisasi Partikel Gelembung
    for (int i = 0; i < MAX_BUBBLES; i++) {
        bubbles[i].x = ((rand() % 1000) / 10.0f) - 50.0f;
        bubbles[i].y = OCEAN_FLOOR + ((rand() % 1000) / 35.0f);
        bubbles[i].z = ((rand() % 1000) / 10.0f) - 50.0f;
        bubbles[i].vx = ((rand() % 100) - 50) / 5000.0f;
        bubbles[i].vy = 0.05f + (rand() % 100) / 1200.0f;
        bubbles[i].vz = ((rand() % 100) - 50) / 5000.0f;
        bubbles[i].size = 0.12f + (rand() % 100) / 500.0f;
        bubbles[i].maxScale = 1.6f + (rand() % 100) / 100.0f;
        bubbles[i].wobblePhase = (rand() % 360) * DEG2RAD;
        bubbles[i].rotAngle = (float)(rand() % 360);
        bubbles[i].rotSpeed = 45.0f + (rand() % 75);
        bubbles[i].rotAxisX = 0.3f + (rand() % 100) / 140.0f;
        bubbles[i].rotAxisY = 0.6f + (rand() % 100) / 120.0f;
        bubbles[i].rotAxisZ = 0.2f + (rand() % 100) / 150.0f;
        bubbles[i].active = true;
    }

    // Inisialisasi Sekolah Ikan Laut
    for (int i = 0; i < NUM_FISH; i++) {
        schoolOfFish[i].centerX = ((i % 3) - 1) * 16.0f;
        schoolOfFish[i].centerY = -16.0f + (i * 2.5f);
        schoolOfFish[i].centerZ = ((i / 3) == 0 ? -12.0f : 12.0f);
        schoolOfFish[i].radiusX = 10.0f + (i * 2.0f);
        schoolOfFish[i].radiusZ = 8.0f + (i * 2.2f);
        schoolOfFish[i].currentAngle = (i * 60.0f) * DEG2RAD;
        schoolOfFish[i].speed = 0.015f + (i * 0.003f);
        schoolOfFish[i].vertAmplitude = 1.2f + (i * 0.3f);
        schoolOfFish[i].vertSpeed = 1.5f + (i * 0.4f);
        schoolOfFish[i].tailWagPhase = (float)(i * 45);
        schoolOfFish[i].sizeScale = 0.85f + (i * 0.1f);

        // Variasi warna ikan tropis laut dalam
        if (i % 3 == 0) {
            schoolOfFish[i].colorR = 0.15f; schoolOfFish[i].colorG = 0.65f; schoolOfFish[i].colorB = 0.95f; // Biru Bahari
        } else if (i % 3 == 1) {
            schoolOfFish[i].colorR = 0.98f; schoolOfFish[i].colorG = 0.50f; schoolOfFish[i].colorB = 0.15f; // Oranye Ikan Badut
        } else {
            schoolOfFish[i].colorR = 0.80f; schoolOfFish[i].colorG = 0.20f; schoolOfFish[i].colorB = 0.70f; // Ungu Bioluminescent
        }
    }

    // Inisialisasi Formasi Batu Mengelilingi Palung Laut
    for (int i = 0; i < NUM_ROCKS; i++) {
        float theta = (i * (360.0f / NUM_ROCKS)) * DEG2RAD;
        float dist = 40.0f + ((i % 4) * 2.5f);
        rocks[i].x = dist * cos(theta);
        rocks[i].z = dist * sin(theta);
        rocks[i].y = OCEAN_FLOOR + 1.5f;
        rocks[i].rotX = (float)(rand() % 45);
        rocks[i].rotY = (float)(rand() % 360);
        rocks[i].rotZ = (float)(rand() % 45);
        rocks[i].scaleX = 2.5f + (rand() % 200) / 100.0f;
        rocks[i].scaleY = 3.2f + (rand() % 300) / 100.0f;
        rocks[i].scaleZ = 2.2f + (rand() % 200) / 100.0f;
    }
}

void initOpenGL() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);

    // Aktifkan Transparansi (Blending) untuk gelembung dan air
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // =========================================================================
    // VISUAL BAWAH LAUT: GL_FOG (Kabut Palung Laut Dalam)
    // =========================================================================
    glEnable(GL_FOG);
    GLfloat fogColor[4] = { 0.015f, 0.065f, 0.14f, 1.0f }; // Deep Abyssal Blue
    glFogi(GL_FOG_MODE, GL_EXP2);
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_DENSITY, 0.022f);
    glHint(GL_FOG_HINT, GL_NICEST);
    glClearColor(fogColor[0], fogColor[1], fogColor[2], 1.0f);

    // =========================================================================
    // PENCAHAYAAN: GL_LIGHT0 (Cahaya Permukaan Samudra / Ambient Blue Filter)
    // =========================================================================
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat light0_ambient[]  = { 0.08f, 0.14f, 0.22f, 1.0f };
    GLfloat light0_diffuse[]  = { 0.25f, 0.50f, 0.65f, 1.0f };
    GLfloat light0_specular[] = { 0.35f, 0.60f, 0.70f, 1.0f };
    GLfloat light0_pos[]      = { 10.0f, 35.0f, 10.0f, 0.0f }; // Arah cahaya dari atas

    glLightfv(GL_LIGHT0, GL_AMBIENT,  light0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light0_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);

    // =========================================================================
    // PENCAHAYAAN: GL_LIGHT1 (Lampu Sorot Kapal Selam / Focused Spotlight)
    // =========================================================================
    glEnable(GL_LIGHT1);
    GLfloat light1_ambient[]  = { 0.05f, 0.05f, 0.05f, 1.0f };
    GLfloat light1_diffuse[]  = { 1.00f, 0.98f, 0.88f, 1.0f }; // Cahaya sorot halogen terang
    GLfloat light1_specular[] = { 1.00f, 1.00f, 0.95f, 1.0f };

    glLightfv(GL_LIGHT1, GL_AMBIENT,  light1_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  light1_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);

    // Karakteristik Sorot (Spotlight Parameters)
    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 32.0f);        // Sudut lebar kerucut sorot
    glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 25.0f);      // Konsentrasi titik pusat sinar
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.015f);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.001f);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

// ==============================================================================
// PEMODELAN OBJEK 3D DENGAN PRIMITIF GLUT (SESUAI SLIDE 02 & 03)
// ==============================================================================

// Helper Material Shading
void setMaterial(float ar, float ag, float ab,
                 float dr, float dg, float db,
                 float sr, float sg, float sb, float shininess) {
    GLfloat amb[]  = { ar, ag, ab, 1.0f };
    GLfloat diff[] = { dr, dg, db, 1.0f };
    GLfloat spec[] = { sr, sg, sb, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, amb);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diff);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
}

// OBJEK 1: KAPAL SELAM PENELITI (Research Submarine)
// Primitif: glutSolidSphere (lambung & jendela), glutSolidCube (dek, periskop, baling-baling), glutSolidCone (lampu sorot)
void drawSubmarine() {
    glPushMatrix();

    // 1. LAMBUNG UTAMA (glutSolidSphere diskalakan elipsoid)
    setMaterial(0.35f, 0.25f, 0.05f, 0.95f, 0.72f, 0.10f, 0.85f, 0.85f, 0.80f, 64.0f);
    glColor3f(0.96f, 0.72f, 0.08f); // Kuning khas kapal selam peneliti
    glPushMatrix();
        glScalef(1.15f, 1.15f, 2.6f);
        glutSolidSphere(1.0, 32, 32);
    glPopMatrix();

    // 2. KOKPIT / JENDELA OBSERVASI DEPAN (glutSolidSphere cembung)
    setMaterial(0.1f, 0.3f, 0.4f, 0.3f, 0.75f, 0.95f, 0.95f, 0.95f, 1.0f, 96.0f);
    glColor3f(0.35f, 0.85f, 0.98f); // Kaca biru cembung
    glPushMatrix();
        glTranslatef(0.0f, 0.1f, 2.2f);
        glScalef(0.75f, 0.75f, 0.65f);
        glutSolidSphere(0.7, 24, 24);
    glPopMatrix();

    // Jendela Samping Kiri & Kanan (Portholes)
    setMaterial(0.2f, 0.2f, 0.2f, 0.3f, 0.7f, 0.9f, 0.9f, 0.9f, 0.9f, 80.0f);
    glColor3f(0.4f, 0.8f, 0.95f);
    for (int side = -1; side <= 1; side += 2) {
        for (float zOff = -0.7f; zOff <= 0.8f; zOff += 0.7f) {
            glPushMatrix();
                glTranslatef(side * 1.05f, 0.2f, zOff);
                glScalef(0.2f, 0.35f, 0.35f);
                glutSolidSphere(0.5, 16, 16);
            glPopMatrix();
        }
    }

    // 3. DEK & MENARA OBSERVASI (glutSolidCube)
    setMaterial(0.25f, 0.20f, 0.05f, 0.85f, 0.60f, 0.05f, 0.6f, 0.6f, 0.6f, 32.0f);
    glColor3f(0.85f, 0.60f, 0.05f);
    glPushMatrix();
        glTranslatef(0.0f, 1.15f, 0.2f);
        glScalef(0.65f, 0.95f, 1.6f);
        glutSolidCube(1.0);
    glPopMatrix();

    // 4. PERISKOP (glutSolidCube vertikal + horizontal)
    setMaterial(0.1f, 0.1f, 0.1f, 0.25f, 0.25f, 0.25f, 0.8f, 0.8f, 0.8f, 50.0f);
    glColor3f(0.25f, 0.25f, 0.28f); // Logam gelap
    glPushMatrix();
        // Tiang Periskop
        glTranslatef(0.0f, 1.95f, 0.5f);
        glPushMatrix();
            glScalef(0.12f, 0.9f, 0.12f);
            glutSolidCube(1.0);
        glPopMatrix();
        // Lensa Periskop
        glTranslatef(0.0f, 0.45f, 0.12f);
        glScalef(0.22f, 0.16f, 0.36f);
        glutSolidCube(1.0);
    glPopMatrix();

    // Lampu Strobe Navigasi Merah di Ujung Periskop
    glPushMatrix();
        glTranslatef(0.0f, 2.5f, 0.5f);
        glColor3f(1.0f, 0.15f, 0.15f);
        glutSolidSphere(0.1, 12, 12);
    glPopMatrix();

    // 5. SIRIP KEMUDI & SAYAP SELAM (glutSolidCube)
    setMaterial(0.3f, 0.2f, 0.05f, 0.9f, 0.65f, 0.08f, 0.5f, 0.5f, 0.5f, 32.0f);
    glColor3f(0.9f, 0.65f, 0.08f);
    // Sirip Horizontal Samping (Diving Wings)
    glPushMatrix();
        glTranslatef(0.0f, -0.1f, 0.2f);
        glScalef(2.6f, 0.08f, 0.75f);
        glutSolidCube(1.0);
    glPopMatrix();
    // Sirip Tegak Ekor (Rudder Fin)
    glPushMatrix();
        glTranslatef(0.0f, 0.7f, -2.2f);
        glScalef(0.1f, 1.1f, 0.8f);
        glutSolidCube(1.0);
    glPopMatrix();

    // 6. MONCONG LAMPU SOROT DEPAN (glutSolidCone)
    setMaterial(0.2f, 0.2f, 0.2f, 0.3f, 0.3f, 0.35f, 0.9f, 0.9f, 0.9f, 64.0f);
    glColor3f(0.35f, 0.35f, 0.38f);
    glPushMatrix();
        glTranslatef(0.0f, -0.45f, 2.2f);
        glutSolidCone(0.38, 0.6, 16, 16);
        // Bohlam Lampu Berpendar
        if (sub.spotlightOn) {
            glColor3f(1.0f, 1.0f, 0.85f);
        } else {
            glColor3f(0.4f, 0.4f, 0.35f);
        }
        glTranslatef(0.0f, 0.0f, 0.45f);
        glutSolidSphere(0.22, 16, 16);
    glPopMatrix();

    // Efek Berkas Kerucut Sinar Sorot Semi-Transparan jika Spotlight ON
    if (sub.spotlightOn) {
        glPushMatrix();
            glDepthMask(GL_FALSE);
            glDisable(GL_LIGHTING);
            glColor4f(1.0f, 0.98f, 0.82f, 0.15f); // Kerucut cahaya transparan
            glTranslatef(0.0f, -0.45f, 2.7f);
            glutSolidCone(4.2, 18.0, 20, 20);
            glEnable(GL_LIGHTING);
            glDepthMask(GL_TRUE);
        glPopMatrix();
    }

    // 7. BALING-BALING BELAKANG DENGAN ISOLASI MATRIKS (Hierarki glPushMatrix)
    // Sesuai slide 02: Isolasi glPushMatrix() untuk rotasi lokal baling-baling
    glPushMatrix();
        glTranslatef(0.0f, 0.0f, -2.65f); // Posisi poros baling-baling di buritan
        glRotatef(sub.propAngle, 0.0f, 0.0f, 1.0f); // Rotasi lokal baling-baling

        // Poros Baling-baling
        setMaterial(0.2f, 0.2f, 0.2f, 0.5f, 0.5f, 0.5f, 0.8f, 0.8f, 0.8f, 64.0f);
        glColor3f(0.45f, 0.45f, 0.48f);
        glutSolidSphere(0.22, 16, 16);

        // 4 Bilah Baling-baling (glutSolidCube)
        glColor3f(0.85f, 0.75f, 0.25f); // Kuningan/Tembaga propeller
        for (int b = 0; b < 4; b++) {
            glPushMatrix();
                glRotatef(b * 90.0f, 0.0f, 0.0f, 1.0f);
                glTranslatef(0.0f, 0.45f, 0.0f);
                glRotatef(20.0f, 1.0f, 0.0f, 0.0f); // Pitch sudut bilah
                glScalef(0.18f, 0.75f, 0.04f);
                glutSolidCube(1.0);
            glPopMatrix();
        }
    glPopMatrix(); // Akhir isolasi rotasi baling-baling

    glPopMatrix();
}

// OBJEK 2: IKAN LAUT (Marine Fish)
// Primitif: glutSolidSphere (badan elipsoid & mata), glutSolidCone (ekor) & primitif sirip
void drawFish(const Fish& f) {
    glPushMatrix();
        glTranslatef(f.posX, f.posY, f.posZ);
        glRotatef(f.yaw * RAD2DEG, 0.0f, 1.0f, 0.0f);
        glScalef(f.sizeScale * marineLifeScale * globalScale,
                 f.sizeScale * marineLifeScale * globalScale,
                 f.sizeScale * marineLifeScale * globalScale);

        // BADAN IKAN ELIPSOID (glutSolidSphere diskalakan)
        setMaterial(0.1f, 0.2f, 0.3f, f.colorR, f.colorG, f.colorB, 0.7f, 0.7f, 0.8f, 48.0f);
        glColor3f(f.colorR, f.colorG, f.colorB);
        glPushMatrix();
            glScalef(0.45f, 0.70f, 1.45f);
            glutSolidSphere(1.0, 20, 20);
        glPopMatrix();

        // MATA IKAN (glutSolidSphere kiri & kanan)
        setMaterial(0.8f, 0.8f, 0.8f, 1.0f, 1.0f, 1.0f, 0.9f, 0.9f, 0.9f, 64.0f);
        glColor3f(1.0f, 1.0f, 1.0f);
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
                glTranslatef(side * 0.42f, 0.20f, 0.85f);
                glutSolidSphere(0.14, 12, 12);
                // Pupil Hitam
                glColor3f(0.02f, 0.02f, 0.02f);
                glTranslatef(side * 0.04f, 0.0f, 0.07f);
                glutSolidSphere(0.08, 10, 10);
            glPopMatrix();
        }

        // SIRIP ATAS (Dorsal Fin)
        glColor3f(f.colorR * 0.8f, f.colorG * 0.8f, f.colorB * 0.8f);
        glPushMatrix();
            glTranslatef(0.0f, 0.85f, -0.1f);
            glRotatef(-25.0f, 1.0f, 0.0f, 0.0f);
            glScalef(0.06f, 0.55f, 0.95f);
            glutSolidCube(1.0);
        glPopMatrix();

        // SIRIP SAMPING DADA (Pectoral Fins mengepak halus)
        float finFlap = sin(globalAnimTime * 8.0f + f.tailWagPhase) * 18.0f;
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
                glTranslatef(side * 0.42f, -0.1f, 0.3f);
                glRotatef(side * (35.0f + finFlap), 0.0f, 0.0f, 1.0f);
                glRotatef(-15.0f, 1.0f, 0.0f, 0.0f);
                glScalef(0.45f, 0.06f, 0.50f);
                glutSolidCube(1.0);
            glPopMatrix();
        }

        // EKOR IKAN DENGAN ISOLASI MATRIKS (Hierarki glPushMatrix)
        // Sesuai slide 02: Isolasi glPushMatrix() untuk rotasi kibasan ekor ikan
        float tailWag = sin(globalAnimTime * 9.0f + f.tailWagPhase) * 26.0f;
        glPushMatrix();
            glTranslatef(0.0f, 0.0f, -1.35f);       // Pangkal ekor
            glRotatef(tailWag, 0.0f, 1.0f, 0.0f);   // Kibasan ekor sumbu Y

            // Pangkal Ekor (glutSolidCone)
            glPushMatrix();
                glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
                glutSolidCone(0.35, 0.8, 14, 14);
            glPopMatrix();

            // Ujung Sirip Ekor (Caudal Fin Mengipas)
            glTranslatef(0.0f, 0.0f, -0.75f);
            glRotatef(tailWag * 0.4f, 0.0f, 1.0f, 0.0f);
            glScalef(0.08f, 0.95f, 0.65f);
            glutSolidCube(1.0);
        glPopMatrix(); // Akhir isolasi ekor

    glPopMatrix();
}

// OBJEK 3: TERUMBU KARANG (Coral Reefs di 8 Titik Koordinat Dasar Laut)
// Primitif: glutSolidSphere (bongkahan karang) + glutSolidCone (cabang karang)
void drawCoral(const Coral& c, int index) {
    glPushMatrix();
        glTranslatef(c.x, c.y, c.z);
        glRotatef(c.rotY, 0.0f, 1.0f, 0.0f);

        // Ayunan Arus Air Laut (Swaying Motion) sesuai slide 04
        float currentSway = sin(globalAnimTime * 2.2f + index * 0.85f) * 4.5f;
        glRotatef(currentSway, 0.0f, 0.0f, 1.0f);

        // Skalasi Cabang & Biota
        glScalef(marineLifeScale * globalScale,
                 c.heightScale * marineLifeScale * globalScale,
                 marineLifeScale * globalScale);

        // Bongkahan Pangkal Karang (glutSolidSphere)
        setMaterial(0.2f, 0.15f, 0.15f, c.colorR * 0.7f, c.colorG * 0.7f, c.colorB * 0.7f, 0.3f, 0.3f, 0.3f, 16.0f);
        glColor3f(c.colorR * 0.75f, c.colorG * 0.75f, c.colorB * 0.75f);
        glPushMatrix();
            glScalef(1.8f, 0.85f, 1.8f);
            glutSolidSphere(1.2, 16, 16);
        glPopMatrix();

        // Rumpun Cabang-cabang Karang (Kombinasi glutSolidCone & glutSolidSphere)
        setMaterial(0.2f, 0.1f, 0.2f, c.colorR, c.colorG, c.colorB, 0.6f, 0.6f, 0.6f, 32.0f);
        glColor3f(c.colorR, c.colorG, c.colorB);

        // Cabang Utama Tengah
        glPushMatrix();
            glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
            glutSolidCone(0.55, 3.8, 12, 12);
            glTranslatef(0.0f, 0.0f, 3.8f);
            glutSolidSphere(0.42, 12, 12);
        glPopMatrix();

        // Cabang-cabang Merekah ke Segala Arah
        for (int branch = 0; branch < 5; branch++) {
            float angle = branch * 72.0f;
            glPushMatrix();
                glRotatef(angle, 0.0f, 1.0f, 0.0f);
                glRotatef(28.0f + (branch % 2) * 12.0f, 1.0f, 0.0f, 0.0f);
                glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
                glutSolidCone(0.38, 2.8 + (branch % 3) * 0.4f, 10, 10);
                glTranslatef(0.0f, 0.0f, 2.8f);
                glutSolidSphere(0.32, 10, 10);
            glPopMatrix();
        }
    glPopMatrix();
}

// OBJEK 4: BATU DASAR LAUT (Sea Rocks Mengelilingi Tepi Palung)
// Primitif: glutSolidDodecahedron dengan kemiringan 3 sumbu & variasi skala pipih/besar
void drawRock(const SeaRock& r) {
    glPushMatrix();
        // Translasi & Rotasi Lingkungan (Q / E)
        glRotatef(envRotation, 0.0f, 1.0f, 0.0f);
        glTranslatef(r.x, r.y, r.z);

        // Kemiringan 3 Sumbu sesuai slide 04
        glRotatef(r.rotX, 1.0f, 0.0f, 0.0f);
        glRotatef(r.rotY, 0.0f, 1.0f, 0.0f);
        glRotatef(r.rotZ, 0.0f, 0.0f, 1.0f);

        // Skalasi Pipih / Besar & Global
        glScalef(r.scaleX * globalScale,
                 r.scaleY * globalScale,
                 r.scaleZ * globalScale);

        // Material Batu Samudra (Matte Basalt Gray/Blue)
        setMaterial(0.12f, 0.15f, 0.18f, 0.32f, 0.38f, 0.42f, 0.25f, 0.28f, 0.30f, 12.0f);
        glColor3f(0.34f, 0.40f, 0.44f);
        glutSolidDodecahedron();
    glPopMatrix();
}

// OBJEK 5: PETI HARTA KARUN & GELEMBUNG (Treasure Chest & Bubbles)
// Primitif: glutSolidCube + glutSolidSphere (Peti), glutSolidSphere (Gelembung)
void drawTreasureChest() {
    glPushMatrix();
        glTranslatef(chest.x, chest.y, chest.z);

        // Putaran Kilau Peti & Denyut Skala sesuai slide 04
        glRotatef(chest.spinAngle, 0.0f, 1.0f, 0.0f);
        float pulseScale = (1.0f + 0.08f * sin(chest.pulseTime * 4.0f)) * globalScale;
        glScalef(pulseScale, pulseScale, pulseScale);

        // 1. KOTAK PETI (glutSolidCube kayu mahoni laut dalam)
        setMaterial(0.3f, 0.15f, 0.05f, 0.55f, 0.28f, 0.10f, 0.4f, 0.3f, 0.2f, 24.0f);
        glColor3f(0.55f, 0.28f, 0.10f);
        glPushMatrix();
            glScalef(1.8f, 1.1f, 1.2f);
            glutSolidCube(1.0);
        glPopMatrix();

        // 2. TUTUP PETI CEMBUNG (glutSolidCube atas)
        setMaterial(0.35f, 0.18f, 0.08f, 0.65f, 0.32f, 0.12f, 0.5f, 0.4f, 0.3f, 24.0f);
        glColor3f(0.62f, 0.30f, 0.12f);
        glPushMatrix();
            glTranslatef(0.0f, 0.65f, 0.0f);
            glScalef(1.85f, 0.45f, 1.25f);
            glutSolidCube(1.0);
        glPopMatrix();

        // 3. LIS SABUK BESI & GEMBOK EMAS (glutSolidSphere)
        // Sabuk Pengikat Logam Emas
        setMaterial(0.3f, 0.25f, 0.05f, 0.95f, 0.82f, 0.15f, 0.9f, 0.85f, 0.6f, 80.0f);
        glColor3f(0.95f, 0.82f, 0.18f); // Emas berkilau
        glPushMatrix();
            glTranslatef(0.0f, 0.15f, 0.62f);
            glutSolidSphere(0.24, 16, 16); // Gembok Emas
        glPopMatrix();

        // Ornamen Sudut Emas Peti
        for (int sx = -1; sx <= 1; sx += 2) {
            for (int sz = -1; sz <= 1; sz += 2) {
                glPushMatrix();
                    glTranslatef(sx * 0.9f, 0.55f, sz * 0.6f);
                    glutSolidSphere(0.12, 10, 10);
                glPopMatrix();
            }
        }

        // Kilau Emas / Harta Karun Memancar di Sekitar Peti
        glPushMatrix();
            glDepthMask(GL_FALSE);
            glDisable(GL_LIGHTING);
            glColor4f(1.0f, 0.85f, 0.25f, 0.35f * (0.5f + 0.5f * sin(chest.pulseTime * 5.0f)));
            glTranslatef(0.0f, 0.7f, 0.0f);
            glutSolidSphere(0.85, 16, 16);
            glEnable(GL_LIGHTING);
            glDepthMask(GL_TRUE);
        glPopMatrix();

    glPopMatrix();
}

// PARTIKEL GELEMBUNG 3D (glutSolidSphere transparan naik membesar dengan rotasi 3D glRotatef)
void drawBubbles() {
    glDepthMask(GL_FALSE); // Transparansi halus
    setMaterial(0.2f, 0.4f, 0.6f, 0.7f, 0.85f, 1.0f, 0.95f, 0.95f, 1.0f, 96.0f);

    for (int i = 0; i < MAX_BUBBLES; i++) {
        if (!bubbles[i].active) continue;
        glPushMatrix();
            // 1. Translasi: Goyangan sinusoidal (wobble) pada sumbu X & Z dan elevasi Y
            float wobbleX = sin(globalAnimTime * 3.0f + bubbles[i].wobblePhase) * 0.25f;
            float wobbleZ = cos(globalAnimTime * 3.0f + bubbles[i].wobblePhase) * 0.25f;
            glTranslatef(bubbles[i].x + wobbleX, bubbles[i].y, bubbles[i].z + wobbleZ);

            // 2. Rotasi: glRotatef 3D per partikel gelembung (Sesuai PPT Slide 03)
            glRotatef(bubbles[i].rotAngle, bubbles[i].rotAxisX, bubbles[i].rotAxisY, bubbles[i].rotAxisZ);

            // 3. Skalasi: Ekspansi ukuran gelembung seiring berkurangnya tekanan air saat naik
            float depthFactor = (bubbles[i].y - OCEAN_FLOOR) / (OCEAN_CEILING - OCEAN_FLOOR);
            if (depthFactor < 0.0f) depthFactor = 0.0f;
            float currentBubbleScale = (1.0f + depthFactor * bubbles[i].maxScale) * globalScale;
            glScalef(currentBubbleScale, currentBubbleScale, currentBubbleScale);

            // Lapisan Dinding Bola Gelembung Transparan
            glColor4f(0.75f, 0.90f, 1.0f, 0.40f);
            glutSolidSphere(bubbles[i].size, 12, 12);

            // Kilau Refleksi / Titik Pantul Cahaya (Specular Glint) pada permukaan gelembung
            // Titik kilau ini ikut berputar bersama rotasi glRotatef sehingga dinamika rotasi terlihat nyata
            glPushMatrix();
                glTranslatef(bubbles[i].size * 0.45f, bubbles[i].size * 0.50f, bubbles[i].size * 0.35f);
                glColor4f(1.0f, 1.0f, 1.0f, 0.80f); // Kilau putih berkilau
                glutSolidSphere(bubbles[i].size * 0.24f, 8, 8);
            glPopMatrix();

        glPopMatrix();
    }
    glDepthMask(GL_TRUE);
}

// DASAR LAUT SAMUDRA / PALUNG LAUT (Seabed Grid Terrain with Caustics Wave)
void drawOceanFloor() {
    glPushMatrix();
        glRotatef(envRotation, 0.0f, 1.0f, 0.0f);

        setMaterial(0.08f, 0.15f, 0.20f, 0.18f, 0.32f, 0.35f, 0.15f, 0.20f, 0.25f, 8.0f);

        const int GRID_SIZE = 40;
        const float GRID_STEP = (WORLD_LIMIT_X * 2.0f) / GRID_SIZE;

        glBegin(GL_QUADS);
        for (int i = 0; i < GRID_SIZE; i++) {
            float x1 = -WORLD_LIMIT_X + i * GRID_STEP;
            float x2 = x1 + GRID_STEP;
            for (int j = 0; j < GRID_SIZE; j++) {
                float z1 = -WORLD_LIMIT_Z + j * GRID_STEP;
                float z2 = z1 + GRID_STEP;

                // Ondulasi kontur pasir samudra & simulasi efek caustics kerlip air
                float y11 = OCEAN_FLOOR + sin(x1 * 0.15f) * cos(z1 * 0.15f) * 0.8f;
                float y21 = OCEAN_FLOOR + sin(x2 * 0.15f) * cos(z1 * 0.15f) * 0.8f;
                float y22 = OCEAN_FLOOR + sin(x2 * 0.15f) * cos(z2 * 0.15f) * 0.8f;
                float y12 = OCEAN_FLOOR + sin(x1 * 0.15f) * cos(z2 * 0.15f) * 0.8f;

                // Varian warna kedalaman palung
                float caustics = 0.5f + 0.5f * sin(x1 * 0.4f + z1 * 0.4f + globalAnimTime * 2.0f);
                glColor3f(0.12f + caustics * 0.05f, 0.24f + caustics * 0.08f, 0.28f + caustics * 0.06f);

                glNormal3f(0.0f, 1.0f, 0.0f);
                glVertex3f(x1, y11, z1);
                glVertex3f(x2, y21, z1);
                glVertex3f(x2, y22, z2);
                glVertex3f(x1, y12, z2);
            }
        }
        glEnd();
    glPopMatrix();
}

// ==============================================================================
// 2D HUD (HEADS-UP DISPLAY) OVERLAY
// ==============================================================================

void renderBitmapString(float x, float y, void *font, const char *string) {
    glRasterPos2f(x, y);
    while (*string) {
        glutBitmapCharacter(font, *string);
        string++;
    }
}

void drawHUD() {
    // Nonaktifkan lighting & depth test untuk render 2D teks overlay
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    float fW = (float)windowWidth;
    float fH = (float)windowHeight;

    // 1. KOTAK HEADER ATAS (Judul Proyek & Identitas Mahasiswa)
    glColor4f(0.02f, 0.08f, 0.16f, 0.75f);
    glBegin(GL_QUADS);
        glVertex2f(15.0f, fH - 15.0f);
        glVertex2f(fW - 15.0f, fH - 15.0f);
        glVertex2f(fW - 15.0f, fH - 85.0f);
        glVertex2f(15.0f, fH - 85.0f);
    glEnd();

    // Teks Header
    glColor3f(0.35f, 0.90f, 1.0f);
    renderBitmapString(28.0f, fH - 38.0f, GLUT_BITMAP_HELVETICA_18,
                       "ABYSS EXPLORER: DEEP SEA ADVENTURE - PRAKTIKUM GRAFIKA KOMPUTER");
    glColor3f(0.85f, 0.88f, 0.92f);
    renderBitmapString(28.0f, fH - 58.0f, GLUT_BITMAP_HELVETICA_12,
                       "1. Frito Radestya S. (241401030) [Kapal, Peti, Spotlight] | 2. M. Al Farel Azhar (241401009) [Biota, Gelembung, Input]");
    renderBitmapString(28.0f, fH - 74.0f, GLUT_BITMAP_HELVETICA_10,
                       "S-1 Ilmu Komputer USU | Dosen: Herriance, S.T., M.Kom.");

    // 2. PANEL TELEMETRI GAMEPLAY (Kiri Bawah)
    glColor4f(0.02f, 0.08f, 0.16f, 0.75f);
    glBegin(GL_QUADS);
        glVertex2f(15.0f, 20.0f);
        glVertex2f(380.0f, 20.0f);
        glVertex2f(380.0f, 240.0f);
        glVertex2f(15.0f, 240.0f);
    glEnd();

    // Border Panel Telemetri
    glColor3f(0.20f, 0.60f, 0.85f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(15.0f, 20.0f);
        glVertex2f(380.0f, 20.0f);
        glVertex2f(380.0f, 240.0f);
        glVertex2f(15.0f, 240.0f);
    glEnd();

    char buf[128];
    // Skor Peti Harta
    glColor3f(1.0f, 0.85f, 0.20f);
    snprintf(buf, sizeof(buf), "[TREASURE] Peti Terkumpul : %d", treasuresCollected);
    renderBitmapString(28.0f, 212.0f, GLUT_BITMAP_HELVETICA_18, buf);

    // Kedalaman (Depth)
    glColor3f(0.35f, 0.90f, 1.0f);
    snprintf(buf, sizeof(buf), "Kedalaman Laut (Depth)  : %.1f m", -sub.y);
    renderBitmapString(28.0f, 185.0f, GLUT_BITMAP_HELVETICA_12, buf);

    // Kecepatan & Status Boost
    snprintf(buf, sizeof(buf), "Kecepatan Kapal Selam  : %.2f knot %s",
             sub.speed * 100.0f, sub.isBoosting ? "[BOOST ACTIVE]" : "");
    renderBitmapString(28.0f, 163.0f, GLUT_BITMAP_HELVETICA_12, buf);

    // Status Lampu Sorot (F1)
    if (sub.spotlightOn) {
        glColor3f(0.4f, 1.0f, 0.4f);
        renderBitmapString(28.0f, 141.0f, GLUT_BITMAP_HELVETICA_12, "Lampu Sorot Kapal (F1) : NYALA (ACTIVE)");
    } else {
        glColor3f(1.0f, 0.35f, 0.35f);
        renderBitmapString(28.0f, 141.0f, GLUT_BITMAP_HELVETICA_12, "Lampu Sorot Kapal (F1) : PADAM (OFF)");
    }

    // Mode Kamera
    glColor3f(0.85f, 0.85f, 0.95f);
    snprintf(buf, sizeof(buf), "Mode Kamera (C)        : %s",
             currentCamMode == CAM_CHASE ? "Follow Submarine (3rd Person)" : "Free Orbit 360 (I/J/K/L)");
    renderBitmapString(28.0f, 119.0f, GLUT_BITMAP_HELVETICA_12, buf);

    // Informasi Skala Geometri
    snprintf(buf, sizeof(buf), "Skala -> Global: %.1fx | Kapal: %.1fx | Biota: %.1fx",
             globalScale, sub.scale, marineLifeScale);
    renderBitmapString(28.0f, 97.0f, GLUT_BITMAP_HELVETICA_12, buf);

    // Status Tabrakan Ikan & Efek Penalti
    if (isPenalized) {
        glColor3f(1.0f, 0.20f, 0.20f);
        snprintf(buf, sizeof(buf), "STATUS BIOTA: BENTURAN! Daya Dorong Turun (-70%%) [%.1fs]", collisionPenaltyTimer);
    } else if (fishCollisionAlert) {
        glColor3f(1.0f, 0.50f, 0.20f);
        snprintf(buf, sizeof(buf), "STATUS BIOTA: AWAS! Menyerempet Ikan (%d kali)", fishCollisionCount);
    } else {
        if (fishCollisionCount > 0) {
            glColor3f(0.40f, 0.85f, 0.95f);
            snprintf(buf, sizeof(buf), "STATUS BIOTA: Aman (Riwayat Benturan: %d kali)", fishCollisionCount);
        } else {
            glColor3f(0.30f, 0.95f, 0.50f);
            snprintf(buf, sizeof(buf), "STATUS BIOTA: Aman / Bebas Rintangan");
        }
    }
    renderBitmapString(28.0f, 75.0f, GLUT_BITMAP_HELVETICA_12, buf);

    // 3. PANEL PANDUAN KONTROL (Kanan Bawah)
    glColor4f(0.02f, 0.08f, 0.16f, 0.75f);
    glBegin(GL_QUADS);
        glVertex2f(fW - 365.0f, 20.0f);
        glVertex2f(fW - 15.0f, 20.0f);
        glVertex2f(fW - 15.0f, 240.0f);
        glVertex2f(fW - 365.0f, 240.0f);
    glEnd();

    glColor3f(0.20f, 0.60f, 0.85f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(fW - 365.0f, 20.0f);
        glVertex2f(fW - 15.0f, 20.0f);
        glVertex2f(fW - 15.0f, 240.0f);
        glVertex2f(fW - 365.0f, 240.0f);
    glEnd();

    glColor3f(1.0f, 0.85f, 0.20f);
    renderBitmapString(fW - 350.0f, 212.0f, GLUT_BITMAP_HELVETICA_12, "PANDUAN KONTROL PROGRAM:");
    glColor3f(0.85f, 0.88f, 0.92f);
    renderBitmapString(fW - 350.0f, 190.0f, GLUT_BITMAP_HELVETICA_10, "Panah Atas / Bawah   : Maju & Mundur");
    renderBitmapString(fW - 350.0f, 172.0f, GLUT_BITMAP_HELVETICA_10, "Panah Kiri / Kanan   : Belok Yaw Kiri / Kanan");
    renderBitmapString(fW - 350.0f, 154.0f, GLUT_BITMAP_HELVETICA_10, "W / S                : Naik & Turun Kedalaman");
    renderBitmapString(fW - 350.0f, 136.0f, GLUT_BITMAP_HELVETICA_10, "A / D | Q / E        : Geser Strafe | Roll & Rotasi");
    renderBitmapString(fW - 350.0f, 118.0f, GLUT_BITMAP_HELVETICA_10, "SPACE                : Kecepatan Boost & Gelembung");
    renderBitmapString(fW - 350.0f, 100.0f, GLUT_BITMAP_HELVETICA_10, "F1                   : Toggle Lampu Sorot Kapal");
    renderBitmapString(fW - 350.0f, 82.0f,  GLUT_BITMAP_HELVETICA_10, "C                    : Ganti Kamera (Chase / Orbit)");
    renderBitmapString(fW - 350.0f, 64.0f,  GLUT_BITMAP_HELVETICA_10, "I / J / K / L        : Putar Kamera Bebas Orbit 360");
    renderBitmapString(fW - 350.0f, 46.0f,  GLUT_BITMAP_HELVETICA_10, "PgUp/Dn | Home/End   : Skala Kapal | Skala Biota");
    renderBitmapString(fW - 350.0f, 28.0f,  GLUT_BITMAP_HELVETICA_10, "+ / -   | R | ESC    : Skala Global | Reset | Keluar");

    // Kembalikan Matrix Mode
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

// ==============================================================================
// UPDATE FISIKA, ANIMASI & LOGIKA TABRAKAN
// ==============================================================================

void updateGame(float dt) {
    globalAnimTime += dt;

    // Update timer cooldown dan durasi penalti tabrakan biota laut
    if (collisionCooldownTimer > 0.0f) {
        collisionCooldownTimer -= dt;
    }
    if (collisionPenaltyTimer > 0.0f) {
        collisionPenaltyTimer -= dt;
        sub.speed *= 0.88f; // Hambatan turbulensi ekstra saat masa pemulihan benturan
        if (collisionPenaltyTimer <= 0.0f) {
            isPenalized = false;
        }
    }

    // 1. Update Posisi & Rotasi Kapal Selam
    // Kecepatan peluruhan (drag / redaman air laut)
    sub.speed *= 0.92f;
    sub.pitch *= 0.88f;

    float forwardX = sin(sub.yaw * DEG2RAD) * cos(sub.pitch * DEG2RAD);
    float forwardZ = cos(sub.yaw * DEG2RAD) * cos(sub.pitch * DEG2RAD);
    float forwardY = -sin(sub.pitch * DEG2RAD);

    sub.x += forwardX * sub.speed;
    sub.z += forwardZ * sub.speed;
    sub.y += forwardY * sub.speed;

    // Batasi dalam palung laut
    if (sub.x < -WORLD_LIMIT_X) sub.x = -WORLD_LIMIT_X;
    if (sub.x >  WORLD_LIMIT_X) sub.x =  WORLD_LIMIT_X;
    if (sub.z < -WORLD_LIMIT_Z) sub.z = -WORLD_LIMIT_Z;
    if (sub.z >  WORLD_LIMIT_Z) sub.z =  WORLD_LIMIT_Z;
    if (sub.y < OCEAN_FLOOR + 2.0f) sub.y = OCEAN_FLOOR + 2.0f;
    if (sub.y > OCEAN_CEILING)     sub.y = OCEAN_CEILING;

    // Putaran baling-baling (berputar kencang bila bergerak atau boost)
    float propSpeed = 720.0f;
    if (sub.isBoosting) propSpeed = 1800.0f;
    sub.propAngle += propSpeed * dt;
    if (sub.propAngle >= 360.0f) sub.propAngle -= 360.0f;

    // 2. Update Partikel Gelembung (Translasi, Rotasi, & Siklus Hidup)
    for (int i = 0; i < MAX_BUBBLES; i++) {
        bubbles[i].y += bubbles[i].vy;
        bubbles[i].x += bubbles[i].vx;
        bubbles[i].z += bubbles[i].vz;

        // Update rotasi sudut 3D gelembung
        bubbles[i].rotAngle += bubbles[i].rotSpeed * dt;
        if (bubbles[i].rotAngle >= 360.0f) bubbles[i].rotAngle -= 360.0f;

        // Reset saat mencapai langit air / permukaan
        if (bubbles[i].y > OCEAN_CEILING) {
            bubbles[i].y = OCEAN_FLOOR + (rand() % 10) * 0.5f;
            bubbles[i].rotAngle = (float)(rand() % 360);
            // Jika kapal sedang boost, semburkan gelembung dari buritan kapal selam
            if (sub.isBoosting && (i % 3 == 0)) {
                bubbles[i].x = sub.x - forwardX * 2.5f + ((rand() % 10) - 5) * 0.1f;
                bubbles[i].y = sub.y - 0.2f + ((rand() % 10) - 5) * 0.1f;
                bubbles[i].z = sub.z - forwardZ * 2.5f + ((rand() % 10) - 5) * 0.1f;
                bubbles[i].vy = 0.12f + (rand() % 100) / 800.0f;
                bubbles[i].rotSpeed = 120.0f + (rand() % 160); // Rotasi lebih cepat saat turbulensi boost
            } else {
                bubbles[i].x = ((rand() % 1000) / 10.0f) - 50.0f;
                bubbles[i].z = ((rand() % 1000) / 10.0f) - 50.0f;
                bubbles[i].vy = 0.05f + (rand() % 100) / 1200.0f;
                bubbles[i].rotSpeed = 45.0f + (rand() % 75);
            }
        }
    }

    // 3. Update Lintasan Renang Ikan Laut (Translasi & Rotasi)
    fishCollisionAlert = false;
    for (int i = 0; i < NUM_FISH; i++) {
        schoolOfFish[i].currentAngle += schoolOfFish[i].speed;
        if (schoolOfFish[i].currentAngle >= 2.0f * M_PI) {
            schoolOfFish[i].currentAngle -= 2.0f * M_PI;
        }

        // Lintasan Melingkar & Vertikal Sinusoidal sesuai slide 04
        schoolOfFish[i].posX = schoolOfFish[i].centerX + schoolOfFish[i].radiusX * cos(schoolOfFish[i].currentAngle);
        schoolOfFish[i].posZ = schoolOfFish[i].centerZ + schoolOfFish[i].radiusZ * sin(schoolOfFish[i].currentAngle);
        schoolOfFish[i].posY = schoolOfFish[i].centerY + schoolOfFish[i].vertAmplitude * sin(schoolOfFish[i].currentAngle * schoolOfFish[i].vertSpeed);

        // Arah hadap renang dihitung dari vektor tangensial lintasan
        float tangentX = -schoolOfFish[i].radiusX * sin(schoolOfFish[i].currentAngle);
        float tangentZ =  schoolOfFish[i].radiusZ * cos(schoolOfFish[i].currentAngle);
        schoolOfFish[i].yaw = atan2(tangentX, tangentZ);

        // Deteksi Tabrakan Kapal Selam dengan Ikan (Bounding Sphere Collision)
        float dx = sub.x - schoolOfFish[i].posX;
        float dy = sub.y - schoolOfFish[i].posY;
        float dz = sub.z - schoolOfFish[i].posZ;
        float distSq = dx * dx + dy * dy + dz * dz;

        float collisionDist = 2.4f * sub.scale * globalScale;
        if (distSq < collisionDist * collisionDist) {
            fishCollisionAlert = true;
            // Picu penalti jika cooldown telah berakhir
            if (collisionCooldownTimer <= 0.0f) {
                fishCollisionCount++;
                isPenalized = true;
                collisionPenaltyTimer = 1.0f;  // Durasi penalti kecepatan 1.0 detik
                collisionCooldownTimer = 1.6f; // Cooldown 1.6 detik mencegah spam tabrakan per frame
                sub.speed *= 0.25f;            // Penalti: kehilangan daya dorong (kecepatan terpangkas 75%)
            }
        }
    }

    // 4. Update Peti Harta Karun (Rotasi & Denyut)
    chest.spinAngle += 35.0f * dt;
    if (chest.spinAngle >= 360.0f) chest.spinAngle -= 360.0f;
    chest.pulseTime += dt;

    // Deteksi Mengambil Peti Harta Karun (Collision Detection)
    float cdx = sub.x - chest.x;
    float cdy = sub.y - chest.y;
    float cdz = sub.z - chest.z;
    float chestDistSq = cdx * cdx + cdy * cdy + cdz * cdz;

    float pickupRadius = 3.2f * sub.scale * globalScale;
    if (chestDistSq < pickupRadius * pickupRadius) {
        treasuresCollected++;
        // Relokasi peti secara acak di dasar laut sesuai slide 04
        relocateTreasure();
    }
}

// ==============================================================================
// CALLBACK DISPLAY & TAMPILAN KAMERA
// ==============================================================================

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // =========================================================================
    // KAMERA 3D (Chase Camera vs Orbit Camera 360)
    // =========================================================================
    if (currentCamMode == CAM_CHASE) {
        // Kamera Mengikuti Kapal Selam (Third Person View)
        float followDist = 8.5f * sub.scale;
        float camHeight  = 2.8f * sub.scale;
        float forwardX   = sin(sub.yaw * DEG2RAD);
        float forwardZ   = cos(sub.yaw * DEG2RAD);

        float camX = sub.x - forwardX * followDist;
        float camY = sub.y + camHeight;
        float camZ = sub.z - forwardZ * followDist;

        float lookTargetX = sub.x + forwardX * 4.0f;
        float lookTargetY = sub.y;
        float lookTargetZ = sub.z + forwardZ * 4.0f;

        gluLookAt(camX, camY, camZ,
                  lookTargetX, lookTargetY, lookTargetZ,
                  0.0f, 1.0f, 0.0f);
    } else {
        // Kamera Bebas Orbit 360 Derajat di Sekitar Kapal Selam
        float radOrbitYaw   = orbitYaw * DEG2RAD;
        float radOrbitPitch = orbitPitch * DEG2RAD;

        float camX = sub.x + orbitDist * cos(radOrbitPitch) * sin(radOrbitYaw);
        float camY = sub.y + orbitDist * sin(radOrbitPitch);
        float camZ = sub.z + orbitDist * cos(radOrbitPitch) * cos(radOrbitYaw);

        gluLookAt(camX, camY, camZ,
                  sub.x, sub.y, sub.z,
                  0.0f, 1.0f, 0.0f);
    }

    // =========================================================================
    // UPDATE POSISI DAN ARAH LAMPU SOROT KAPAL SELAM (GL_LIGHT1)
    // =========================================================================
    if (sub.spotlightOn) {
        glEnable(GL_LIGHT1);

        float forwardX = sin(sub.yaw * DEG2RAD) * cos(sub.pitch * DEG2RAD);
        float forwardY = -sin(sub.pitch * DEG2RAD);
        float forwardZ = cos(sub.yaw * DEG2RAD) * cos(sub.pitch * DEG2RAD);

        // Posisi lampu di moncong depan kapal selam
        GLfloat spotPos[] = {
            sub.x + forwardX * (2.2f * sub.scale),
            sub.y - 0.45f * sub.scale + forwardY * (2.2f * sub.scale),
            sub.z + forwardZ * (2.2f * sub.scale),
            1.0f // Positional Light
        };
        GLfloat spotDir[] = { forwardX, forwardY, forwardZ };

        glLightfv(GL_LIGHT1, GL_POSITION, spotPos);
        glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spotDir);
    } else {
        glDisable(GL_LIGHT1);
    }

    // =========================================================================
    // RENDERING SELURUH ELEMEN DUNIA 3D
    // =========================================================================

    // 1. Dasar Laut Palung Samudra & Efek Caustics
    drawOceanFloor();

    // 2. Formasi Batu Mengitari Palung (Primitif Dodecahedron)
    for (int i = 0; i < NUM_ROCKS; i++) {
        drawRock(rocks[i]);
    }

    // 3. Terumbu Karang di 8 Titik Koordinat (Primitif Cone + Sphere)
    for (int i = 0; i < NUM_CORALS; i++) {
        drawCoral(corals[i], i);
    }

    // 4. Kawanan Ikan Laut Berenang (Primitif Sphere + Cone)
    for (int i = 0; i < NUM_FISH; i++) {
        drawFish(schoolOfFish[i]);
    }

    // 5. Peti Harta Karun (Primitif Cube + Sphere)
    drawTreasureChest();

    // 6. Kapal Selam Peneliti (Primitif Sphere, Cube, Cone)
    glPushMatrix();
        glTranslatef(sub.x, sub.y, sub.z);
        glRotatef(sub.yaw,   0.0f, 1.0f, 0.0f);
        glRotatef(sub.pitch, 1.0f, 0.0f, 0.0f);
        glRotatef(sub.roll,  0.0f, 0.0f, 1.0f);
        glScalef(sub.scale * globalScale,
                 sub.scale * globalScale,
                 sub.scale * globalScale);
        drawSubmarine();
    glPopMatrix();

    // 7. Partikel Gelembung Udara 3D
    drawBubbles();

    // 8. Heads-Up Display (HUD 2D)
    drawHUD();

    glutSwapBuffers();
}

// ==============================================================================
// CALLBACK INPUT KEYBOARD (SESUAI SLIDE 05)
// ==============================================================================

// Callback Tombol Khusus (glutSpecialFunc)
void specialKeys(int key, int x, int y) {
    switch (key) {
        // Navigasi Maju & Mundur
        case GLUT_KEY_UP:
            if (isPenalized) {
                // Selama masa penalti, daya dorong baling-baling terbatas
                sub.speed += 0.08f;
                if (sub.speed > 0.28f) sub.speed = 0.28f;
            } else {
                sub.speed += 0.22f;
                if (sub.speed > 0.85f) sub.speed = 0.85f;
            }
            break;
        case GLUT_KEY_DOWN:
            sub.speed -= 0.16f;
            if (sub.speed < -0.45f) sub.speed = -0.45f;
            break;

        // Navigasi Belok Yaw Kiri & Kanan
        case GLUT_KEY_LEFT:
            sub.yaw += 4.5f;
            if (sub.yaw >= 360.0f) sub.yaw -= 360.0f;
            break;
        case GLUT_KEY_RIGHT:
            sub.yaw -= 4.5f;
            if (sub.yaw < 0.0f) sub.yaw += 360.0f;
            break;

        // Skala Objek Kapal Selam (PageUp / PageDown)
        case GLUT_KEY_PAGE_UP:
            sub.scale += 0.08f;
            if (sub.scale > 2.5f) sub.scale = 2.5f;
            break;
        case GLUT_KEY_PAGE_DOWN:
            sub.scale -= 0.08f;
            if (sub.scale < 0.5f) sub.scale = 0.5f;
            break;

        // Skala Objek Biota Laut: Ikan & Karang (Home / End)
        case GLUT_KEY_HOME:
            marineLifeScale += 0.08f;
            if (marineLifeScale > 2.5f) marineLifeScale = 2.5f;
            break;
        case GLUT_KEY_END:
            marineLifeScale -= 0.08f;
            if (marineLifeScale < 0.5f) marineLifeScale = 0.5f;
            break;

        // Toggle Lampu Sorot Kapal (F1)
        case GLUT_KEY_F1:
            sub.spotlightOn = !sub.spotlightOn;
            break;
    }
    glutPostRedisplay();
}

// Callback Tombol Biasa (glutKeyboardFunc)
void normalKeys(unsigned char key, int x, int y) {
    switch (key) {
        // Naik & Turun Kedalaman (W / S)
        case 'w':
        case 'W':
            sub.y += 0.45f;
            sub.pitch = -12.0f; // Dongak ke atas
            if (sub.y > OCEAN_CEILING) sub.y = OCEAN_CEILING;
            break;
        case 's':
        case 'S':
            sub.y -= 0.45f;
            sub.pitch = 12.0f;  // Menukik ke bawah
            if (sub.y < OCEAN_FLOOR + 2.0f) sub.y = OCEAN_FLOOR + 2.0f;
            break;

        // Geser Kiri & Kanan / Strafe (A / D)
        case 'a':
        case 'A': {
            float strafeX = cos(sub.yaw * DEG2RAD);
            float strafeZ = -sin(sub.yaw * DEG2RAD);
            sub.x -= strafeX * 0.45f;
            sub.z -= strafeZ * 0.45f;
            break;
        }
        case 'd':
        case 'D': {
            float strafeX = cos(sub.yaw * DEG2RAD);
            float strafeZ = -sin(sub.yaw * DEG2RAD);
            sub.x += strafeX * 0.45f;
            sub.z += strafeZ * 0.45f;
            break;
        }

        // Miring Roll & Rotasi Lingkungan (Q / E)
        case 'q':
        case 'Q':
            sub.roll -= 4.0f;
            envRotation -= 2.0f;
            break;
        case 'e':
        case 'E':
            sub.roll += 4.0f;
            envRotation += 2.0f;
            break;

        // Kecepatan Boost & Gelembung Ekstra (Space)
        case ' ':
            sub.isBoosting = true;
            if (isPenalized) {
                // Boost diredam selama masa pemulihan benturan
                sub.speed += 0.15f;
                if (sub.speed > 0.45f) sub.speed = 0.45f;
            } else {
                sub.speed += 0.45f;
                if (sub.speed > 1.4f) sub.speed = 1.4f;
            }
            break;

        // Skala Semua Objek / Skala Global (+ / -)
        case '+':
        case '=':
            globalScale += 0.08f;
            if (globalScale > 2.5f) globalScale = 2.5f;
            break;
        case '-':
        case '_':
            globalScale -= 0.08f;
            if (globalScale < 0.4f) globalScale = 0.4f;
            break;

        // Rotasi Kamera Bebas Orbit 360 (I / J / K / L)
        case 'i':
        case 'I':
            orbitPitch += 3.5f;
            if (orbitPitch > 85.0f) orbitPitch = 85.0f;
            break;
        case 'k':
        case 'K':
            orbitPitch -= 3.5f;
            if (orbitPitch < -45.0f) orbitPitch = -45.0f;
            break;
        case 'j':
        case 'J':
            orbitYaw += 4.5f;
            break;
        case 'l':
        case 'L':
            orbitYaw -= 4.5f;
            break;

        // Ganti Mode Kamera (C)
        case 'c':
        case 'C':
            if (currentCamMode == CAM_CHASE) {
                currentCamMode = CAM_ORBIT;
            } else {
                currentCamMode = CAM_CHASE;
            }
            break;

        // Reset Simulasi & State (R)
        case 'r':
        case 'R':
            initGame();
            break;

        // Keluar Program (ESC)
        case 27:
            exit(0);
            break;
    }
    glutPostRedisplay();
}

void keyUp(unsigned char key, int x, int y) {
    if (key == ' ') {
        sub.isBoosting = false;
    }
}

// ==============================================================================
// CALLBACK WINDOW RESHAPE & TIMER ANIMASI
// ==============================================================================

void reshape(int w, int h) {
    if (h == 0) h = 1;
    windowWidth = w;
    windowHeight = h;

    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(55.0, (GLdouble)w / (GLdouble)h, 0.5, 200.0);

    glMatrixMode(GL_MODELVIEW);
}

void timer(int value) {
    updateGame(0.016f); // ~60 FPS
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

// ==============================================================================
// ENTRY POINT UTAMA (main)
// ==============================================================================

int main(int argc, char** argv) {
    srand((unsigned int)time(NULL));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(100, 50);
    glutCreateWindow("Abyss Explorer: Deep Sea Adventure - Praktikum Grafika Komputer USU");

    initOpenGL();
    initGame();

    // Register GLUT Callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(specialKeys);
    glutKeyboardFunc(normalKeys);
    glutKeyboardUpFunc(keyUp);
    glutTimerFunc(16, timer, 0);

    printf("=================================================================\n");
    printf(" ABYSS EXPLORER: DEEP SEA ADVENTURE\n");
    printf(" Proyek Akhir Praktikum Grafika Komputer\n");
    printf(" 1. Frito Radestya S.        (241401030)\n");
    printf(" 2. Muhammad Al Farel Azhar (241401009)\n");
    printf(" S-1 Ilmu Komputer - Universitas Sumatera Utara\n");
    printf("=================================================================\n");
    printf(" Kontrol:\n");
    printf(" - Panah Atas/Bawah  : Maju / Mundur\n");
    printf(" - Panah Kiri/Kanan  : Belok Yaw\n");
    printf(" - W / S             : Kedalaman Naik / Turun\n");
    printf(" - A / D             : Strafe Kiri / Kanan\n");
    printf(" - Q / E             : Roll Miring & Rotasi Lingkungan\n");
    printf(" - SPACE             : Kecepatan Boost & Gelembung Ekstra\n");
    printf(" - F1                : Toggle Lampu Sorot (Spotlight GL_LIGHT1)\n");
    printf(" - C                 : Ganti Mode Kamera (Chase / Orbit 360)\n");
    printf(" - I / J / K / L     : Putar Kamera Orbit Bebas\n");
    printf(" - PageUp / PageDown : Skala Kapal Selam\n");
    printf(" - Home / End        : Skala Biota Laut (Ikan & Karang)\n");
    printf(" - + / -             : Skala Global Semua Objek\n");
    printf(" - R                 : Reset Simulasi & Posisi\n");
    printf(" - ESC               : Keluar Program\n");
    printf("=================================================================\n");

    glutMainLoop();
    return 0;
}
