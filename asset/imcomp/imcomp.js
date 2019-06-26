////////////////////////////////////////////////////////////////////////////////
//
// @file        imcomp.js
// @description Image comparator user interface maintainers and handlers
// @author      Abhishek Dutta <adutta@robots.ox.ac.uk>
// @date        11 July 2018
//
////////////////////////////////////////////////////////////////////////////////

var IMCOMP_PANEL_NAME = { STEP1:'step1',
			  									STEP2:'step2',
			  									STEP3:'step3',
                          STEP4:'step4'};

var IMCOMP_THEME_MESSAGE_TIMEOUT_MS    = 6000;

var _imcomp_about = {};
_imcomp_about.name = "Image Comparator";
_imcomp_about.shortname = "imcomp";
_imcomp_about.version = "1.0.0";
_imcomp_about.date_first_release  = "July 11, 2017";
_imcomp_about.date_current_release  = "";
_imcomp_about.author_name  = "Abhishek Dutta";
_imcomp_about.author_email = "adutta@robots.ox.ac.uk";

var _imcomp_toggle_timer = {};
var _imcomp_compare_button_anim_timer = {};
_via_message_clear_timer = {};

function _imcomp_user_interface() {
  // see https://developer.apple.com/library/content/documentation/General/Conceptual/DevPedia-CocoaCore/MVC.html
  this.m = new _imcomp_model();
  this.v = new _imcomp_view();
  this.c = new _imcomp_controller();

  // name, version, etc defined in _model.js

  this.init = function() {
    console.log("Initializing imcomp user interface components ...");
    this.m.init( this.c );
    this.v.init( this.c );
    this.c.init( this.m, this.v );

		/*
		// // DEBUG:
		console.log('************************************ _imcomp_set_panel ACTIVE')
		_imcomp.m.file_count = 3;
	  _imcomp_set_panel(IMCOMP_PANEL_NAME.STEP3);
		document.getElementById('compare_panel').style.display = '';
		document.getElementById('ref_line_container').style.display = '';
		document.getElementById('right_content').style.display = '';
	  document.getElementById('files_panel').style.display = '';
	  document.getElementById('tab_tools_panel').classList.add('display-none');
	  document.getElementById('toggle_controls').classList.add('display-none');
	  document.getElementById('fade_controls').classList.add('display-none');
		document.getElementById('results_tabs_panel').style.display = 'none';

	  document.getElementById('top_panel').classList.remove('display-none');
	  document.getElementById('banner').style.display = 'none';
		*/
  }
}

var _imcomp = new _imcomp_user_interface();
_imcomp.init();

function _imcomp_init() {
  console.log('Initializing imcomp ...');
  init_message_panel();
  _imcomp_set_panel(IMCOMP_PANEL_NAME.STEP1, false);
  show_message('Image Comparator ' + _imcomp_about.version);
}

// sets the right panel based on panel_id
// is_navigation is true if we are navigating back and forth using back or home buttons
function _imcomp_set_panel(panel_id, is_navigation) {

	if ( panel_id === IMCOMP_PANEL_NAME.STEP1 ) {
		document.getElementById('top_right').style.display = 'none';
		document.getElementById('step1_text').style.border = '';
	}

  if ( panel_id === IMCOMP_PANEL_NAME.STEP3 ) {

    if ( _imcomp.m.file_count < 2 ) {
      show_message('To compare, you must add at least two images!');
      _imcomp_set_panel(IMCOMP_PANEL_NAME.STEP1, false);
      return;
    }
		document.getElementById('top_right').style.display = '';
		show_message('Please draw a region with your mouse on the left image and click compare. Or click compare to compare the whole image.');
  }

  // update all buttons
  var blist = document.getElementsByClassName('panel_selector_button');
  var n = blist.length;
  var i;
  for ( i = 0; i < n; ++i ) {
    if ( blist[i].id.startsWith(panel_id) ) {
      blist[i].classList.add('stepn_button_activate');
      blist[i].classList.remove('stepn_button_deactivate');
      //console.log('activate ' + blist[i].id);
    } else {
      blist[i].classList.add('stepn_button_deactivate');
      blist[i].classList.remove('stepn_button_activate');
      document.getElementById(panel_id + '_panel').style.display = 'none';
      //console.log('deactivate ' + blist[i].id);
    }
  }

  // update all content
  var clist = document.getElementsByClassName('panel_content');
  var n = clist.length;
  var i;
  for ( i = 0; i < n; ++i ) {
    if ( clist[i].id.startsWith(panel_id) ) {
      clist[i].style.display = 'block';
      _imcomp_set_panel_content(panel_id, is_navigation);
    } else {
      clist[i].style.display = 'none';
    }
  }
	// highlight instructions for the page
	_imcomp.c.brighten_instructions(panel_id);
}

function _imcomp_set_panel_content(panel_id, is_navigation) {
  if ( panel_id === IMCOMP_PANEL_NAME.STEP2 ) {
    var cs = document.getElementById('comparison_settings');
    cs.innerHTML = '';
    cs.appendChild( _imcomp_get_comparison_settings_algname_list() );
    cs.appendChild( _imcomp_get_comparison_settings_default_settings_option() );
    _imcomp_get_comparison_settings_onchange_algname();
  }

  if ( panel_id === IMCOMP_PANEL_NAME.STEP3 ) {
    var img_index = 0;
    console.log(_imcomp.c.type_list)
    for ( var type in _imcomp.c.type_list ) {
      _imcomp.c.update_view_filelist(type);
      _imcomp.c.set_now(type, img_index);
      img_index = img_index + 1;

      var sid_suffix = type + '_via';
      _imcomp.c.content_selector_set_state(type, sid_suffix, true);
      _imcomp.c.enable_switch(type, '_zoom');
    }

		// show all uploaded files in the files panel
		document.getElementById('files_panel_header').classList.remove('display-none');
		document.getElementById('contents_panel_header').classList.remove('display-none');
		var fp = document.getElementById('files_panel');
		if ( !is_navigation ) {
		  for ( var i = 0; i < _imcomp.m.files.length; i++ ) {
				var fr = new FileReader();
				// same for base and comp
				fr.fid = _imcomp.m.fid_to_via_fileid.base[i];
				fr.f_idx = i;
		    fr.addEventListener( 'load', function(e) {
		    	var img = document.createElement('img');
					img.setAttribute('draggable', true);
					// img.setAttribute('id', e.currentTarget.fid);
					img.setAttribute('id', 'files_panel_file_' + e.currentTarget.f_idx);
		    	img.setAttribute('src', e.currentTarget.result);
		    	fp.appendChild(img);
		    }.bind(this));

		    fr.readAsDataURL( _imcomp.m.files[i] );
			}
	  }
		_imcomp.c.format_comparison_page();
  }
	// result of comparison panel
	if (panel_id === IMCOMP_PANEL_NAME.STEP4 ) {
		_imcomp.c.enable_results_tabs();
		_imcomp.c.format_results_page();
	}
}

function _imcomp_get_comparison_settings_onchange_algname() {
  // clear existing alg_description
  var cs = document.getElementById('comparison_settings');
  var existing_desc = cs.getElementsByClassName('alg_description');
  while ( existing_desc.length ) {
    existing_desc[0].parentNode.removeChild( existing_desc[0] );
  }
  // remove existing options (if any)
  var existing_options = cs.getElementsByClassName('comparison_alg_config');
  while ( existing_options.length ) {
    existing_options[0].parentNode.removeChild( existing_options[0] );
  }

  var l = document.getElementById('algname_list');
  var algid = l.options[l.selectedIndex].value;
  var algindex = _imcomp_get_algname_list_index(algid);
  var algdesc = document.createElement('span');
  algdesc.classList.add('value');
  var p1 = document.createElement('p');
  p1.innerHTML = _imcomp_algname_list[algindex].description;
  algdesc.appendChild(p1);

  var ol = document.createElement('ul');
  var i, li;
  var n = _imcomp_algname_list[algindex].references.length;
  for ( i = 0; i < n; ++i ) {
    li = document.createElement('li');
    li.innerHTML = _imcomp_algname_list[algindex].references[i];
    ol.appendChild(li);
  }
  algdesc.appendChild(ol);
  var p2 = document.createElement('p');
  p2.innerHTML = 'Developer and Maintainer: ' + _imcomp_algname_list[algindex].author_name.join(', ');
  algdesc.appendChild(p2);

  var div = document.createElement('div');
  div.classList.add('row');
  div.classList.add('alg_description');
  var span = document.createElement('span');
  span.classList.add('key');
  span.innerHTML = 'Description';
  div.appendChild(span);
  div.appendChild(algdesc);
  cs.appendChild(div);

  _imcomp_get_comparison_settings_onchange_default_settings();
}

function _imcomp_get_comparison_settings_onchange_value(e) {
  _imcomp.c.compare_config[e.target.id] = e.target.value;

  var alg_index = parseInt(e.target.getAttribute('data-alg-index'));
  var config_index = parseInt(e.target.getAttribute('data-config-index'));
  _imcomp_algname_list[alg_index].config[config_index]['value'] = e.target.value;
}

function _imcomp_get_comparison_settings_onchange_default_settings(e) {
  var cs = document.getElementById('comparison_settings');

  // remove existing options (if any)
  var existing_options = cs.getElementsByClassName('comparison_alg_config');
  while ( existing_options.length ) {
    existing_options[0].parentNode.removeChild( existing_options[0] );
  }

  var cbox = document.getElementById('alg_use_default_settings');
  if ( ! cbox.checked ) {
    // show all options
    var l = document.getElementById('algname_list');
    var algid = l.options[l.selectedIndex].value;
    var algindex = _imcomp_get_algname_list_index(algid);
    var i, ci;
    var n = _imcomp_algname_list[algindex].config.length;

    var div, span1, span2, input;
    for ( i = 0; i < n; ++i ) {
      ci = _imcomp_algname_list[algindex].config[i];
      div = document.createElement('div');
      div.classList.add('row');
      div.classList.add('comparison_alg_config');
      span1 = document.createElement('span');
      span1.classList.add('key');
      span1.classList.add('config');
      span1.innerHTML = ci.name;
      span1.setAttribute('title', ci.description);

      input = document.createElement('input');
      input.setAttribute('type', 'text');
      input.setAttribute('id', ci.name);
      input.setAttribute('value', ci.value);
      input.setAttribute('title', ci.description);
      input.setAttribute('data-alg-index', algindex);
      input.setAttribute('data-config-index', i);
      input.addEventListener('change', _imcomp_get_comparison_settings_onchange_value);

      span2 = document.createElement('span');
      span2.classList.add('value');
      span2.appendChild(input);

      div.appendChild(span1);
      div.appendChild(span2);
      cs.appendChild(div);

      _imcomp.c.compare_config[ci.name] = ci.value;
    }
  }
}

function _imcomp_get_algname_list_index(algid) {
  var i, n;
  n = _imcomp_algname_list.length;
  for ( i = 0; i < n; ++i ) {
    if ( _imcomp_algname_list[i].id === algid ) {
      return i;
    }
  }
}

function _imcomp_get_comparison_settings_algname_list() {
  var r1 = document.createElement('div');
  r1.classList.add('row');
  var s1 = document.createElement('span');
  s1.classList.add('key');
  s1.innerHTML = 'Method';
  var s2 = document.createElement('span');
  s2.classList.add('value');
  var sel = document.createElement('select');
  sel.setAttribute('id', 'algname_list');
  sel.addEventListener('change', _imcomp_get_comparison_settings_onchange_algname);

  var o, n, i;
  n = _imcomp_algname_list.length;
  for ( i = 0; i < n; ++i ) {
    o = document.createElement('option');
    o.setAttribute('value', _imcomp_algname_list[i].id);
    o.innerHTML = '[' + (i+1) + '] ' + _imcomp_algname_list[i].name;
    sel.appendChild(o);
  }
  s2.appendChild(sel);

  r1.appendChild(s1);
  r1.appendChild(s2);
  return r1;
}

function _imcomp_get_comparison_settings_default_settings_option() {
  var r1 = document.createElement('div');
  r1.classList.add('row');
  var s1 = document.createElement('span');
  s1.classList.add('key');
  s1.innerHTML = 'Use Default Settings';
  var s2 = document.createElement('span');
  s2.classList.add('value');
  var cbox = document.createElement('input');
  cbox.setAttribute('id', 'alg_use_default_settings');
  cbox.setAttribute('type', 'checkbox');
  cbox.setAttribute('checked', 'checked');
  cbox.addEventListener('change', _imcomp_get_comparison_settings_onchange_default_settings);
  s2.appendChild(cbox);

  r1.appendChild(s1);
  r1.appendChild(s2);
  return r1;
}

// message panel
function init_message_panel() {
  var p = document.getElementById('message_panel');
  p.addEventListener('mousedown', function() {
    this.style.display = 'none';
  }, false);
  p.addEventListener('mouseover', function() {
    clearTimeout(_via_message_clear_timer); // stop any previous timeouts
  }, false);
}

function show_message(msg, t) {
  if ( _via_message_clear_timer ) {
    clearTimeout(_via_message_clear_timer); // stop any previous timeouts
  }
  var timeout = t;
  if ( typeof t === 'undefined' ) {
    timeout = IMCOMP_THEME_MESSAGE_TIMEOUT_MS;
  }
  document.getElementById('message_panel_content').innerHTML = msg;
  document.getElementById('message_panel').style.display = 'block';

  _via_message_clear_timer = setTimeout( function() {
    document.getElementById('message_panel').style.display = 'none';
  }, timeout);
}
