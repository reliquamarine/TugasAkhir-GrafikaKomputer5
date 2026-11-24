# Abyss Explorer: Deep Sea Adventure
**Proyek Akhir Praktikum Grafika Komputer (OpenGL 3D & FreeGLUT - C++)**  
**S-1 Ilmu Komputer - Universitas Sumatera Utara**  
**Dosen Pengampu:** Herriance, S.T., M.Kom.

---

## Tim Pengembang
1. **Frito Radestya S. (241401030)**  
   *Fokus:* Pemodelan 3D Kapal Selam & Peti Harta Karun, Isolasi Matriks Baling-baling, Sistem Pencahayaan Dual (`GL_LIGHT0` & `GL_LIGHT1` Spotlight).
2. **Muhammad Al Farel Azhar (241401009)**  
   *Fokus:* Pemodelan Biota Laut (Kawanan Ikan, 8 Titik Terumbu Karang, Formasi Batu Palung), Partikel Gelembung 3D, Sistem Interaktivitas & Navigasi Input.

---

## Ringkasan Fitur Sesuai Rancangan Proposal (PPT)

### 1. Visual & Atmosfer Bawah Laut Realistis
- **Kabut Palung Laut Dalam (`GL_FOG`)**: Mode `GL_EXP2` dengan warna *abyssal indigo* (`0.015, 0.065, 0.14`) menciptakan gradasi kedalaman samudra.
- **Pencahayaan Permukaan (`GL_LIGHT0`)**: Sumber cahaya ambient & directional dari atas permukaan laut dengan filter biru kehijauan.
- **Lampu Sorot Kapal Selam (`GL_LIGHT1` Spotlight)**: Lampu sorot halogen yang menempel di haluan kapal selam dengan sudut kerucut (*cutoff* 32°), eksponen sorot 25.0, atenuasi jarak, dan berkas sinar volumetrik semi-transparan. Dapat dinyalakan/dimatikan dengan tombol `F1`.
- **Dasar Laut & Efek Caustics**: Kontur palung bergelombang dengan kerlip riak cahaya dinamis (*caustics wave*).

### 2. Pemodelan 5 Objek 3D Primitif GLUT (Slide 02 & 03)
1. **Kapal Selam Peneliti (Research Submarine)**:
   - Lambung utama & jendela pengamat: `glutSolidSphere` (elipsoid & bola cembung).
   - Menara dek observasi, periskop, bilah baling-baling, dan sirip kemudi: `glutSolidCube`.
   - Dudukan moncong lampu sorot depan: `glutSolidCone` + bola lampu berpendar.
   - Baling-baling 4 bilah di buritan berputar dinamis dengan isolasi matriks `glPushMatrix()`.
2. **Kawanan Ikan Laut (Marine Biota Fish)**:
   - Badan elipsoid memanjang: `glutSolidSphere`.
   - Mata ikan (sclera putih + pupil hitam): `glutSolidSphere`.
   - Ekor mengayun (*tail wagging*) & sirip renang: `glutSolidCone` & `glutSolidCube` yang bergerak sinusoidal diisolasi dengan `glPushMatrix()`.
3. **Terumbu Karang (Coral Reefs)**:
   - Tersebar presisi di 8 koordinat dasar laut palung.
   - Kombinasi bongkahan dasar `glutSolidSphere` dan cabang-cabang mekar `glutSolidCone` beraneka warna.
   - Mengayun lembut mengikuti arus air (*swaying motion*).
4. **Batu Dasar Laut (Sea Rocks)**:
   - Mengitari perimeter palung samudra.
   - Primitif `glutSolidDodecahedron` dengan kemiringan 3 sumbu dan variasi skala pipih/besar.
   - Merespons rotasi lingkungan via tombol `Q`/`E`.
5. **Peti Harta Karun & Gelembung 3D**:
   - **Peti Emas**: Kotak peti `glutSolidCube`, tutup lengkung, sabuk logam, dan gembok emas `glutSolidSphere`. Berputar anggun (*spin*) dan berdenyut (*pulse*).
   - **Gelembung Udara 3D**: Partikel `glutSolidSphere` semi-transparan yang melayang naik, membesar seiring berkurangnya tekanan air laut, bergoyang alami, dan menyembur deras saat tombol `SPACE` (*boost*) ditekan.

### 3. Transformasi Geometri Lengkap (Slide 04)
- **Translasi (`glTranslatef`)**: Navigasi kapal bebas X/Y/Z, lintasan renang ikan melingkar & vertikal, relokasi peti acak saat diambil, gelembung naik.
- **Rotasi (`glRotatef`)**: Belok Yaw, menukik Pitch, miring Roll, putaran baling-baling kapal, kibasan ekor ikan, ayunan arus karang, kemiringan batu, putaran kilau peti.
- **Skalasi (`glScalef`)**: Skala kapal selam (`PgUp`/`PgDn`), skala biota laut (`Home`/`End`), skala global (`+`/`-`), elipsoid ikan, ekspansi gelembung.

### 4. Sistem Kamera Ganda
- **Mode 1: Follow Chase Camera (Third Person)**: Mengikuti pergerakan kapal selam secara halus dan sinematis.
- **Mode 2: Free Orbit Camera 360°**: Mengitari kapal selam bebas dari sudut pandang manapun menggunakan tombol `I`/`J`/`K`/`L`.
- **Toggle**: Ditekan tombol `C` untuk beralih mode kapan saja.

### 5. Mekanik Permainan & 2D HUD (Heads-Up Display)
- **Deteksi Tabrakan Peti Harta Karun**: Jika kapal selam mendekati peti harta karun (< radius jangkauan), skor bertambah dan peti berelokasi secara otomatis ke koordinat acak baru di dasar laut.
- **Peringatan Tabrakan Biota Ikan**: Mendeteksi tabrakan fisik antara kapal selam dengan kawanan ikan laut dan menampilkan indikator peringatan bahaya di layar.
- **HUD 2D Terpadu**:
  - Judul proyek & identitas mahasiswa.
  - Jumlah peti harta karun terkumpul.
  - Indikator kedalaman (*Depth meter*).
  - Kecepatan navigasi & status *Boost*.
  - Status Lampu Sorot (`F1`: NYALA / PADAM).
  - Mode Kamera aktif.
  - Pengali skala geometri (*Global, Submarine, Biota*).
  - Panel bantuan tombol navigasi.

---

## Panduan Kontrol Lengkap

| Tombol | Fungsi |
| :--- | :--- |
| **Panah Atas / Bawah** | Maju / Mundur (*Forward / Reverse*) |
| **Panah Kiri / Kanan** | Belok Hadap Kiri / Kanan (*Yaw Rotation*) |
| **W / S** | Naik / Turun Kedalaman Laut (*Depth Pitch*) |
| **A / D** | Geser Samping Kiri / Kanan (*Strafe*) |
| **Q / E** | Miring Kapal (*Roll*) & Rotasi Lingkungan Batu |
| **SPACE** | Kecepatan Penuh (*Boost*) & Semburan Gelembung Baling-baling |
| **F1** | Nyalakan / Matikan Lampu Sorot Kapal (*GL_LIGHT1 Spotlight*) |
| **C** | Ganti Mode Kamera (*Follow Chase* <-> *Free Orbit 360°*) |
| **I / K** | Kamera Orbit Menunduk / Mendongak (*Pitch Up / Down*) |
| **J / L** | Kamera Orbit Putar Kiri / Kanan (*Yaw Left / Right*) |
| **Page Up / Page Down** | Perbesar / Perkecil Ukuran Kapal Selam |
| **Home / End** | Perbesar / Perkecil Ukuran Biota Laut (Ikan & Terumbu Karang) |
| **+ / -** | Skala Global Semua Objek Dunia 3D |
| **R** | Reset Posisi, Simulasi & Skor Permainan |
| **ESC** | Keluar dari Aplikasi |

---

## Petunjuk Menjalankan di Visual Studio

1. Buka file **`AbyssExplorer.sln`** (atau **`AbyssExplorer.slnx`**) di Visual Studio.
2. Pastikan target konfigurasi disetel ke **`Debug`** atau **`Release`** dengan platform **`x86`** (Win32).
3. Tekan **`F5`** (atau **`Ctrl + F5`**) untuk mengompilasi dan langsung menjalankan program.
4. *Catatan Portabilitas:* Seluruh pustaka FreeGLUT (`freeglut.h`, `glut.h`, `freeglut.lib`, `freeglut.dll`) telah disertakan langsung di dalam folder proyek, sehingga proyek ini bersifat mandiri (*self-contained*). `freeglut.dll` otomatis disalin ke folder output lewat *Post-Build Event*.

### Troubleshooting

- **"unresolved external symbol __imp_glutInit..." / "cannot start debugging"**  
  Pastikan platform di toolbar Visual Studio adalah **x86**, bukan **x64**. Library `freeglut.lib` yang disertakan hanya versi 32-bit, sehingga konfigurasi x64 sengaja tidak disediakan.
- **"The code execution cannot proceed because freeglut.dll was not found"**  
  Lakukan **Build > Rebuild Solution**; Post-Build Event akan menyalin DLL ke `Debug\` / `Release\`.
- **Error C4996 (sprintf)**  
  Sudah ditangani dengan `#define _CRT_SECURE_NO_WARNINGS` di baris pertama `main.cpp`.
