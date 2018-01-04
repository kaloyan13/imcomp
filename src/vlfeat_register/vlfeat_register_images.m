close all; clear; clc;

% run('/data/mybin/vlfeat/matlab_install/vlfeat-0.9.20/toolbox/vl_setup')

fn1 = '/home/tlm/dev/imcomp/src/vlfeat_register/a.pgm';
fn2 = '/home/tlm/dev/imcomp/src/vlfeat_register/b.pgm';

im1 = single(imread(fn1));
im2 = single(imread(fn2));

fprintf(1, '\n\nfile1\n');
[f1,d1] = vl_sift(im1, 'verbose', 'verbose');
fprintf(1, '\n\nfile2\n');
[f2,d2] = vl_sift(im2, 'verbose', 'verbose');

return
[matches, ~] = vl_ubcmatch(d1,d2);

numMatches = size(matches,2) ;


X1 = f1(1:2,matches(1,:)) ; X1(3,:) = 1 ;
X2 = f2(1:2,matches(2,:)) ; X2(3,:) = 1 ;

subset = vl_colsubset(1:numMatches, 4) ;
A = [] ;
for i = subset
  A = cat(1, A, kron(X1(:,i)', vl_hat(X2(:,i)))) ;
end

for t = 1:10000
  % estimate homograpyh
  subset = vl_colsubset(1:numMatches, 4) ;
  A = [] ;
  for i = subset
    A = cat(1, A, kron(X1(:,i)', vl_hat(X2(:,i)))) ;
  end
  [U,S,V] = svd(A) ;
  H{t} = reshape(V(:,9),3,3) ;

  % score homography
  X2_ = H{t} * X1 ;
  du = X2_(1,:)./X2_(3,:) - X2(1,:)./X2(3,:) ;
  dv = X2_(2,:)./X2_(3,:) - X2(2,:)./X2(3,:) ;
  ok{t} = (du.*du + dv.*dv) < 3*3 ;
  score(t) = sum(ok{t}) ;
end

[score, best] = max(score) ;
H = H{best} ;
ok = ok{best} ;

return
box2 = [1  size(im2,2) size(im2,2)  1 ;
            1  1           size(im2,1)  size(im2,1) ;
            1  1           1            1 ] ;
box2_ = inv(H) * box2 ;
box2_(1,:) = box2_(1,:) ./ box2_(3,:) ;
box2_(2,:) = box2_(2,:) ./ box2_(3,:) ;
ur =  1:size(im1,2);
vr =  1:size(im1,1);

[u,v] = meshgrid(ur,vr) ;
im1_ = vl_imwbackward(im2double(im1),u,v) ;

z_ = H(3,1) * u + H(3,2) * v + H(3,3) ;
u_ = (H(1,1) * u + H(1,2) * v + H(1,3)) ./ z_ ;
v_ = (H(2,1) * u + H(2,2) * v + H(2,3)) ./ z_ ;
im2_ = vl_imwbackward(im2double(im2),u_,v_) ;

%% find overlap

im1_(isnan(im1_)) = 0 ;
im2_(isnan(im2_)) = 0 ;
mosaic = (im1_ + im2_) ./ 2 ;