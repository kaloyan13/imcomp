/*
  Image Comparator (IMCOMP)
  www.robots.ox.ac.uk/~vgg/software/imcomp/

  Copyright (c) 2017-2018, Abhishek Dutta, Visual Geometry Group, Oxford University.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.
  Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

////////////////////////////////////////////////////////////////////////////////
//
// @file        traherne.js
// @description Traherne Digital Collator user interface javascript
// @author      Abhishek Dutta <adutta@robots.ox.ac.uk>
// @date        Nov 2017
//
////////////////////////////////////////////////////////////////////////////////

function traherne_imcomp_user_interface() {
  // see https://developer.apple.com/library/content/documentation/General/Conceptual/DevPedia-CocoaCore/MVC.html
  this.m = new _traherne_model();
  this.v = new _traherne_view();
  this.c = new _traherne_controller();

  // name, version, etc defined in _model.js

  this.init = function() {
    //console.log("Initializing Traherne user interface components ...");
    this.m.init( this.c );
    this.v.init( this.c );
    this.c.init( this.m, this.v );
  }
}

var _traherne_about = {};
_traherne_about.name = "Traherne Digital Collator";
_traherne_about.shortname = "traherne";
_traherne_about.version = "2.0.4";
_traherne_about.date_first_release  = "June 01, 2017";
_traherne_about.date_current_release  = "";
_traherne_about.author_name  = "Abhishek Dutta";
_traherne_about.author_email = "adutta@robots.ox.ac.uk";

var _traherne_toggle_timer = {};
var _traherne_compare_button_anim_timer = {};

var _traherne_imcomp_user_interface = new traherne_imcomp_user_interface();
_traherne_imcomp_user_interface.init();
