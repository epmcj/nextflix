%https://www.bogotobogo.com/Matlab/Matlab_Tutorial_Video_Processing_1_Object_Detection_by_Color_Thresholding.php
%wavelet transform?
%http://fourier.eng.hmc.edu/e161/lectures/svdcompression.html
n = 45;

obj = VideoReader('sample.mp4'); %abrir o objeto de vídeo

img = double(read(obj,10000));%ler o quadro 10000 do vídeo[altura][largura][rgb]

R = img(:,:,1); G = img(:,:,2); B = img(:,:,3);

[Ur,Sr,Vr]=svd(R);
[Ug,Sg,Vg]=svd(G);
[Ub,Sb,Vb]=svd(B);

Sr(n:end,:) = 0*Sr(n:end,:);
Sg(n:end,:) = 0*Sg(n:end,:);
Sb(n:end,:) = 0*Sb(n:end,:);

R = Ur*Sr*Vr';
G = Ug*Sg*Vg';
B = Ub*Sb*Vb';

img(:,:,1) = R; img(:,:,2) = G; img(:,:,3) = B;

imshow(uint8(img));%display uma imagem
