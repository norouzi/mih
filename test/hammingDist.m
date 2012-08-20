function Dh = hammingDist(B1, B2)
%
% Compute hamming distance between two sets of binary codes (B1, B2)
%
% Dh = hammingDist(B1, B2);
%
% Input:
%    B1, B2: compact bit vectors. Each binary code is one column comprising of uint8 numbers.
%    size(B1) = [nwords, ndatapoints1]
%    size(B2) = [nwords, ndatapoints2]
%    It is faster if ndatapoints2 < ndatapoints1
% 
% Output:
%    Dh = hamming distance. 
%    size(Dh) = [ndatapoints1, ndatapoints2]

% example query:
%    Dhamm = hammingDist(B2, B1);
%    size(Dhamm) = [Ntest x Ntraining]

% look-up table:
bit_in_char = uint16([...
    0 1 1 2 1 2 2 3 1 2 2 3 2 3 3 4 1 2 2 3 2 3 ...
    3 4 2 3 3 4 3 4 4 5 1 2 2 3 2 3 3 4 2 3 3 4 ...
    3 4 4 5 2 3 3 4 3 4 4 5 3 4 4 5 4 5 5 6 1 2 ...
    2 3 2 3 3 4 2 3 3 4 3 4 4 5 2 3 3 4 3 4 4 5 ...
    3 4 4 5 4 5 5 6 2 3 3 4 3 4 4 5 3 4 4 5 4 5 ...
    5 6 3 4 4 5 4 5 5 6 4 5 5 6 5 6 6 7 1 2 2 3 ...
    2 3 3 4 2 3 3 4 3 4 4 5 2 3 3 4 3 4 4 5 3 4 ...
    4 5 4 5 5 6 2 3 3 4 3 4 4 5 3 4 4 5 4 5 5 6 ...
    3 4 4 5 4 5 5 6 4 5 5 6 5 6 6 7 2 3 3 4 3 4 ...
    4 5 3 4 4 5 4 5 5 6 3 4 4 5 4 5 5 6 4 5 5 6 ...
    5 6 6 7 3 4 4 5 4 5 5 6 4 5 5 6 5 6 6 7 4 5 ...
    5 6 5 6 6 7 5 6 6 7 6 7 7 8]);

n1 = size(B1, 2);
[nwords n2] = size(B2);

Dh = zeros([n1 n2], 'uint16');

for j = 1:n2
  for n = 1:nwords
    y = uint16(bitxor(B1(n,:),B2(n,j))'); % y+1 could become 256, which causes an error for uint8 y
    Dh(:,j) = Dh(:,j) + bit_in_char(y+1)';
  end
end
