function [fn,fnSub]=generateFileName(dataset, graph, theta, alpha, motif_id)

fn=['../../data_' dataset '/analysis/' graph '/' num2str(theta) ...
    '/diff-' num2str(alpha) '/m' num2str(motif_id) '.txt'];

if nargout==2;
    fnSub=['../../data_' dataset '/analysis/' graph '/' num2str(theta) ...
        '/diff-' num2str(alpha) '/sub.txt'];
end

end
