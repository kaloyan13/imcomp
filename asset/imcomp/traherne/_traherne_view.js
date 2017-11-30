function _traherne_view() {
  this.now = {};
  this.now['base'] = {};
  this.now['comp'] = {};

  this.cpanel = document.getElementById('top_panel'); // global control panel


  _traherne_view.prototype.init = function( traherne_controller ) {
    this.c = traherne_controller;
    this.connect_ui_elements_to_traherne_view();
  }

  // capture all user interface events and notify _traherne_controller
  this.handleEvent = function(e) {
    if( e.currentTarget.classList.contains('disabled') ) {
      return;
    }
    switch(e.currentTarget.id) {
    case 'base_load_images':
      this.select_local_files('base');
      break;
    case 'base_move_to_next':
      this.c.move_to_next('base');
      break;
    case 'base_move_to_prev':
      this.c.move_to_prev('base');
      break;
    default:
      console.log('_via_view: handler unknown for event: ' + e.currentTarget);
    }
    e.stopPropagation();

  }
}

_traherne_view.prototype.select_local_files = function(type) {
  this.local_file_selector = document.createElement('input');
  this.local_file_selector.setAttribute('id', 'local_file_selector');
  this.local_file_selector.setAttribute('type', 'file');
  this.local_file_selector.setAttribute('name', 'files[]');
  this.local_file_selector.setAttribute('multiple', 'multiple');
  this.local_file_selector.setAttribute('style', 'display:none;');
  //this.local_file_selector.classList.add('display-none');
  this.local_file_selector.setAttribute('accept', '.jpg,.jpeg,.png,.bmp,.tif,.tiff');

  if( type === 'base' || type === 'comp' ) {
    this.local_file_selector.addEventListener('change', function(e) {
      console.log(e);
      console.log(type);
      console.log(this.c);
      this.c.update_files(type, e);
    }.bind(this), false);
    this.local_file_selector.click();
  } else {
    this.local_file_selector = {};
    console.log('unknown type : ' + type);
  }
}

_traherne_view.prototype.connect_ui_elements_to_traherne_view = function() {
  document.getElementById('base_load_images').addEventListener('click', this, false);
  document.getElementById('base_move_to_prev').addEventListener('click', this, false);
  document.getElementById('base_move_to_next').addEventListener('click', this, false);
  document.getElementById('base_img_filename_list').addEventListener('change', this.c.base_now_update.bind(this.c), false);
  document.getElementById('comp_img_filename_list').addEventListener('change', this.c.comp_now_update.bind(this.c), false);
}
