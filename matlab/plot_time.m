function figure1 = plot_time(mih_file, linscan_file, figfname, logp, xlimit, ylimit, small)

mih = load(mih_file);
linscan = load(linscan_file);

mih

if (numel(mih.nMs) ~= numel(linscan.nMs) || any(mih.nMs ~= linscan.nMs))
  error('nMs are not the same in the two files.');
end
nMs = mih.nMs;
ret = mih.ret;
linscan = linscan.linscan;


if ~exist('small') || ~small
  axfsize = 18;
  fsize = 26;
else
  axfsize = 25;
  fsize = 33;
end


if (~exist('small') || small < 2)
  figure1 = figure('XVisual',...
		   '0x12c (TrueColor, depth 24, RGB mask 0xff0000 0xff00 0x00ff)',...
		   'Position', [0 0 620 480]);
else
  figure1 = figure('XVisual',...
		   '0x12c (TrueColor, depth 24, RGB mask 0xff0000 0xff00 0x00ff)',...
		   'Position', [0 0 600 480]);
end


if exist('logp') && logp
  axes1 = axes('Parent', figure1, 'YGrid', 'off', 'FontSize', axfsize);
else
  axes1 = axes('Parent', figure1, 'YGrid', 'on', 'FontSize', axfsize);
end


box(axes1, 'on');
hold(axes1, 'all');

n1 = numel([ret(1,:,1).cput]);
n10 = numel([ret(10,:,1).cput]);
n100 = numel([ret(100,:,1).cput]);
n1000 = numel([ret(1000,:,1).cput]);
nlin = numel([linscan(:).cput]);

if exist('logp') && logp
  set(gca,'xscale','log');
  set(gca,'yscale','log');
  p(7) = plot(nMs,   [logp*sqrt(nMs)], 'k--', 'LineWidth', 2, 'MarkerSize', 9, 'MarkerFaceColor','k','DisplayName', 'sqrt(n)');
end

if ~exist('small') || ~small
  p(1) = plot(nMs(1:n1),    mean(reshape([ret(1,1:n1,:).cput], [n1 size(ret,3)]), 2),          'mv-', 'MarkerFaceColor','m','LineWidth', 3, 'MarkerSize', 9, 'DisplayName', '1-NN');
end
p(2) = plot(nMs(1:n10),   mean(reshape([ret(10,1:n10,:).cput], [n10 size(ret,3)]), 2),       'bd-', 'MarkerFaceColor','b','LineWidth', 3, 'MarkerSize', 9, 'DisplayName', '10-NN');
p(3) = plot(nMs(1:n100),  mean(reshape([ret(100,1:n100,:).cput], [n100 size(ret,3)]), 2),    'Color',[0 .6 0],'Marker','^', 'MarkerFaceColor',[0 .6 0],'LineWidth', 3, 'MarkerSize', 9, 'DisplayName', '100-NN');
p(4) = plot(nMs(1:n1000), mean(reshape([ret(1000,1:n1000,:).cput], [n1000 size(ret,3)]), 2), 'r.-', 'MarkerFaceColor','r','LineWidth', 3, 'MarkerSize', 32, 'DisplayName', '1000-NN');
p(5) = plot(nMs(1:nlin),   [linscan(1:nlin).cput], 'ks-', 'LineWidth', 3, 'MarkerSize', 9, 'MarkerFaceColor','k','DisplayName', 'Linear scan');

if (exist('xlimit') && ~isempty(xlimit))
  xlim(xlimit);
end
if (exist('ylimit') && ~isempty(ylimit))
  ylim(ylimit);
end

if exist('logp') && logp
  ylabel('log time per query (log_{10} s)', 'fontsize', fsize, 'interpreter', 'tex');
  
  xtick = floor(min(log10(nMs * 10^6))):ceil(max(log10(nMs * 10^6)));
  set(gca,'XTick', 10.^(xtick-6));
  set(gca,'XTickLabel', xtick);
  
  ytick = [-4 -3 -2 -1 0 1];
  set(gca, 'YTick', 10.^ytick);
  set(gca, 'YTickLabel', ytick);
  % plot([0,10^max(xtick)*10^6],[9,9], 'k--');
else
  if ~exist('small') || small < 2
    ylabel('time per query (s)', 'fontsize', fsize);
  end

  if exist('small') && small
    xtick = [200 500 1000];
    set(gca,'XTick', xtick);
  end
end
  

if exist('logp') && logp
  xlabel('dataset size (log_{10})', 'fontsize', fsize, 'interpreter', 'tex');
else
  xlabel('dataset size (millions)', 'fontsize', fsize);
end

% legend('Location', 'NorthWest');
if exist('logp') && logp
  if ~exist('small') || ~small
    legend(p([5:-1:1 7]), 'Location', 'NorthWest');
    % set(legend, 'interpreter', 'latex');
  else
    legend(p([5:-1:2 7]), 'Location', 'NorthWest');
  end
else
  if ~exist('small') || ~small
    legend(p([5:-1:1]), 'Location', 'NorthWest');
    % set(legend, 'interpreter', 'latex');
  else
    legend(p([5:-1:2]), 'Location', 'NorthWest');
  end
end

if (exist('figfname') && ~isempty(figfname))
  exportfig(figure1, figfname, 'Color', 'rgb');
  eval(['!epstopdf "', figfname, '" -o="', figfname(1:end-4), '.pdf"']);
end



function script
plot_time('cache/mih_64_1B.mat', 'cache/linscan_64_1B.mat');
plot_time('cache/mih_64_1B.mat', 'cache/linscan_64_1B.mat', [], 0, [.01,1000], [0,.1]);
plot_time('cache/mih_64_1B.mat', 'cache/linscan_64_1B.mat', [], .0002, [.01 1000], [0 20]);
