var _imcomp_algname_list = [];

var ransac_dlt = {};
ransac_dlt.id           = 'ransac_dlt';
ransac_dlt.short_name   = 'ransac_dlt';
ransac_dlt.name         = 'RANSAC with Direct Linear Transform';
ransac_dlt.description  = 'RANSAC with Direct Linear Transform';
ransac_dlt.author_name  = ['Abhishek Dutta'];
ransac_dlt.author_email = ['adutta@robots.ox.ac.uk'];
ransac_dlt.references   = [];
ransac_dlt.references.push('Hartley, R., & Zisserman, A. (2003). Multiple view geometry in computer vision. (Chapter 4)');
ransac_dlt.config       = [];
ransac_dlt.config.push({ 'name':'lowe_2nn_threshold', 'description':'Threshold for Lowe 2nd nearest neighbour test', 'value':1.5, 'type':'text', 'advanced':true});
ransac_dlt.config.push({ 'name':'ransac_iteration_count', 'description':'Number of RANSAC iterations (1 implies corresponds to number of putative matches)', 'value':0.6, 'type':'text', 'advanced':true});
_imcomp_algname_list.push(ransac_dlt);

var robust_ransac_tps = {};
robust_ransac_tps.id           = 'robust_ransac_tps';
robust_ransac_tps.short_name   = 'robust_ransac_tps';
robust_ransac_tps.name         = 'Thin Plate Spline based on Robust Matching';
robust_ransac_tps.description  = 'Robust matching of features followed by thin plate spline based transformation';
robust_ransac_tps.author_name  = ['Abhishek Dutta'];
robust_ransac_tps.author_email = ['adutta@robots.ox.ac.uk'];
robust_ransac_tps.references   = [];
robust_ransac_tps.references.push('Tran, Q.H., Chin, T.J., Carneiro, G., Brown, M.S. and Suter, D., 2012. In defence of RANSAC for outlier rejection in deformable registration.');
robust_ransac_tps.references.push('Bookstein, F.L., 1989. Principal warps: Thin-plate splines and the decomposition of deformations.');
robust_ransac_tps.config       = [];
robust_ransac_tps.config.push({ 'name':'lowe_2nn_threshold', 'description':'Threshold for Lowe 2nd nearest neighbour test', 'value':1.5, 'type':'text', 'advanced':true });
robust_ransac_tps.config.push({ 'name':'ransac_iteration_count', 'description':'Number of RANSAC iterations (1 implies corresponds to number of putative matches)', 'value':0.6, 'type':'text', 'advanced':true });
robust_ransac_tps.config.push({ 'name':'robust_ransac_threshold', 'description':'threshold for robust RANSAC', 'value':0.09, 'type':'text', 'advanced':true });
robust_ransac_tps.config.push({ 'name':'tps_grid_columns', 'description':'number of columns of TPS grid', 'value':'auto', 'type':'text', 'advanced':true });
robust_ransac_tps.config.push({ 'name':'tps_grid_rows', 'description':'number of rows of TPS grid', 'value':'auto', 'type':'text', 'advanced':true });
robust_ransac_tps.config.push({ 'name':'lambda', 'description':'factor to ensure diagonal terms of matrix K are not 0', 'value':'0.001', 'type':'text', 'advanced':true });
_imcomp_algname_list.push(robust_ransac_tps);
