% Test whether the results of multi-index hashing are consistent with the results of linear scan.
% Because the k'th nearest neighbor is not necessarily unique, only the value of the Hamming distances to the neighbors are compared.

function test_mih_with_linscan(code_file, B_over_8, mih_file, linscan_file)

fprintf ('Loading binary codes... ');
load(code_file);			% includes codes as B, and queries as Q
fprintf('done.\n');

mih = load(mih_file);
linscan = load(linscan_file);

if (numel(mih.nMs) ~= numel(linscan.nMs) || any(mih.nMs ~= linscan.nMs))
  error('nMs are not the same in the two files.');
end
nMs = mih.nMs;


for inM = 1:numel(nMs)
  N = nMs(inM) * 1e6;
  fprintf('N = %.1e:\t', N);
  
  for R=[1 10 100 1000]
    NQ = 1000;
    if (isempty(mih.ret(R, inM).res))
      fprintf('No mih for R=%d\t', R, 1e6*nMs(inM));
      continue;
    end
    
    if (NQ ~= linscan.linscan(inM).nq)
      NQ = linscan.linscan(inM).nq;
      fprintf('NQ = %d\t', NQ);
    end
    
    for q=1:NQ  
      mih_ret = mih.ret(R, inM).res(:,q);
      
      dists_mih = hammingDist(B(1:B_over_8, mih_ret), Q(1:B_over_8, q));
      if (any(dists_mih ~= sort(dists_mih)))
	error('mih results are not sorted in Hamming distance!');
      end
           
      if (mih.ret(R, inM).stat(5,q) ~= dists_mih(end))
	fprintf('largest Hamming distance does not match the statistics returned (%d ~= %d)', mih.ret(R, inM).stat(5,q), dists_mih(end));
	error('');
      end
      
      if (~isempty(linscan.linscan(inM).res))
	lin_ret = linscan.linscan(inM).res(:,q);
      
	dists_lin = hammingDist(B(1:B_over_8, lin_ret), Q(1:B_over_8, q));
	R2 = numel(dists_lin);
	
	d1 = dists_lin(1:min(R,R2));
	d2 = dists_mih(1:min(R,R2));
	
	diff = d1 ~= d2;
	if (any(diff))
	  d1(diff)'
	  d2(diff)'
	  error('mih distances are not consistent with linscan distances');
	end
      end
    end
  end
  fprintf('\n');
end
