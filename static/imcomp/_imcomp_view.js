function _imcomp_view() {
  this.compare_panel = document.getElementById('compare_panel');
  this.select_files_panel = document.getElementById('select_files_panel');

  this.now = {};
  this.now.toggle_interval_ms = 0;

  // notify _via_ctrl if user interaction events
  this.handleEvent = function(e) {
    if( e.currentTarget.classList.contains('disabled') ) {
      return;
    }

    switch(e.currentTarget.id) {
    case 'add_file_local':
      this.c.select_local_files();
      break;

    case 'compare':
      this.c.compare();
      break;

    case 'toolbox_view_overlay':
      this.c.show_compare_result();
      break;

    case 'toolbox_view_difference':
      this.c.show_difference_result();
      break;

    case 'toolbox_view_toggle':
      this.c.compare_result_toggle();
      break;

    case 'file1_zoom_in':
      this.c.image1_via.c.zoom_activate();
      break;
    case 'file1_zoom_reset':
      this.c.image1_via.c.zoom_deactivate();
      break;
    case 'file1_delete_sel_region':
      this.c.image1_via.c.delete_selected_regions();
      break;

    case 'file2_zoom_in':
      this.c.image2_via.c.zoom_activate();
      break;
    case 'file2_zoom_reset':
      this.c.image2_via.c.zoom_deactivate();
      break;
    case 'file2_delete_sel_region':
      this.c.image2_via.c.delete_selected_regions();
      break;

    case 'toolbox_download_button':
      var name = document.getElementById('toolbox_download_select').value;
      this.c.download_compare_asset(name);
      break;

    default:
      console.log('_via_view: handler unknown for event: ' + e.currentTarget);
    }
    e.stopPropagation();
  }
}

_imcomp_view.prototype.init = function( imcomp_ctrl ) {
  //console.log('Initializing _imcomp_view ...');
  this.c = imcomp_ctrl;
  this.init_local_file_selector();
  this.register_ui_action_handlers();
}

_imcomp_view.prototype.init_local_file_selector = function() {
  this.local_file_selector = document.createElement('input');
  this.local_file_selector.setAttribute('id', 'local_file_selector');
  this.local_file_selector.setAttribute('type', 'file');
  this.local_file_selector.setAttribute('name', 'files[]');
  this.local_file_selector.setAttribute('multiple', 'multiple');
  this.local_file_selector.classList.add('display-none');
  this.local_file_selector.setAttribute('accept', '.jpg,.jpeg,.png,.bmp,.tif,.tiff');
  this.local_file_selector.addEventListener('change',
                                            this.c.add_user_sel_local_files.bind(this.c),
                                            false);

  this.select_files_panel.appendChild(this.local_file_selector);
}

_imcomp_view.prototype.register_ui_action_handlers = function() {
  // register _imcomp_view as the handler for all user interaction events
  // after receiving these events, _imcomp_view.handleEvent() notifies
  // _imcomp_ctrl to handle these events

  // add file local
  document.getElementById('add_file_local').addEventListener('click', this, false);
  document.getElementById('compare').addEventListener('click', this, false);

  document.getElementById('toolbox_view_difference').addEventListener('click', this, false);
  document.getElementById('toolbox_view_overlay').addEventListener('click', this, false);
  document.getElementById('toolbox_view_toggle').addEventListener('click', this, false);

  document.getElementById('file1_delete_sel_region').addEventListener('click', this, false);
  document.getElementById('file2_delete_sel_region').addEventListener('click', this, false);

  document.getElementById('file1_zoom_in').addEventListener('click', this, false);
  document.getElementById('file1_zoom_reset').addEventListener('click', this, false);
  document.getElementById('file2_zoom_in').addEventListener('click', this, false);
  document.getElementById('file2_zoom_reset').addEventListener('click', this, false);

  document.getElementById('toolbox_download_button').addEventListener('click', this, false);
}

_imcomp_view.prototype.enable_all_toolbox_buttons = function() {
  // enable all tools button
  document.getElementById('toolbox_view_difference').classList.remove('disabled');
  document.getElementById('toolbox_view_overlay').classList.remove('disabled');
  document.getElementById('toolbox_view_toggle').classList.remove('disabled');

  document.getElementById('toolbox_download_select').disabled = false;
  document.getElementById('toolbox_download_button').classList.remove('disabled');
}

_imcomp_view.prototype.disable_all_toolbox_buttons = function() {
  // disable all tools button
  document.getElementById('toolbox_view_difference').classList.add('disabled');
  document.getElementById('toolbox_view_overlay').classList.add('disabled');
  document.getElementById('toolbox_view_toggle').classList.add('disabled');

  document.getElementById('toolbox_download_select').disabled = true;
  document.getElementById('toolbox_download_button').classList.add('disabled');
}

_imcomp_view.prototype.toolbox_view_clear_all_pressed = function() {
  document.getElementById('toolbox_view_difference').classList.remove('pressed');
  document.getElementById('toolbox_view_overlay').classList.remove('pressed');
  document.getElementById('toolbox_view_toggle').classList.remove('pressed');
}
