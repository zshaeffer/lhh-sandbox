/*
Copyright (c) 2003-2011, CKSource - Frederico Knabben. All rights reserved.
For licensing, see LICENSE.html or http://ckeditor.com/license
*/

/*
 * This file is used/requested by the 'Styles' button.
 * 'Styles' button is not enabled by default in DrupalFull and DrupalFiltered toolbars.
 */

CKEDITOR.addStylesSet( 'drupal',
[
	/* Block Styles */

	// These styles are already available in the "Format" combo, so they are
	// not needed here by default. You may enable them to avoid placing the
	// "Format" combo in the toolbar, maintaining the same features.
	
	{ name : 'Paragraph'		, element : 'p' },
	//{ name : 'Heading 1'		, element : 'h1' },
	{ name : 'Heading 2'		, element : 'h2' },
	{ name : 'Heading 3'		, element : 'h3' },
	{ name : 'Heading 4'		, element : 'h4' },
	{ name : 'Heading 5'		, element : 'h5' },
	{ name : 'Heading 6'		, element : 'h6' },
	{ name : 'Preformatted Text', element : 'pre' },
	{ name : 'Address'			, element : 'address' },
	{ name : 'Blockquote'		, element : 'blockquote' },

	/* Object Styles */ 
	{ name : 'Status Box'	, element : 'div', attributes : {'class' : 'status-box'}},
	{ name : 'Warming Box'	, element : 'div', attributes : {'class' : 'warning-box'}},
	{ name : 'Error Box'	, element : 'div', attributes : {'class' : 'error-box'}},
	{ name : 'Info Box'		, element : 'div', attributes : {'class' : 'info-box'}},
	{ name : 'Sidebar Box'		, element : 'div', attributes : {'class' : 'sidebar-box'}},
	{ name : 'Typo Title'		, element : 'div', attributes : {'class' : 'typo-title'}},
	/* Inline Styles */
	{ name : 'Download'	, element : 'p', attributes : {'class' : 'download'}},
	{ name : 'Upload'	, element : 'p', attributes : {'class' : 'upload'}},
	{ name : 'Note'		, element : 'p', attributes : {'class' : 'note'}},
	{ name : 'Tip'		, element : 'p', attributes : {'class' : 'tip'}},
	{ name : 'Word'		, element : 'p', attributes : {'class' : 'word'}},
	{ name : 'Excel'	, element : 'p', attributes : {'class' : 'excel'}},
	{ name : 'PDF'		, element : 'p', attributes : {'class' : 'pdf'}},
	{ name : 'Compressed', element : 'p', attributes : {'class' : 'compressed'}},
	{ name : 'User'		, element : 'p', attributes : {'class' : 'user'}},
	{ name : 'Comment'	, element : 'p', attributes : {'class' : 'comment'}},
	{ name : 'Alert'	, element : 'p', attributes : {'class' : 'alert'}},
	{ name : 'Info'		, element : 'p', attributes : {'class' : 'info'}},
	{ name : 'Check'	, element : 'p', attributes : {'class' : 'check'}},
	{ name : 'Arrow'	, element : 'p', attributes : {'class' : 'arrow'}},
	{ name : 'Star'		, element : 'p', attributes : {'class' : 'star'}},
	{ name : 'Email'	, element : 'p', attributes : {'class' : 'email'}},
	{ name : 'Mobile'	, element : 'p', attributes : {'class' : 'mobile'}},
	{ name : 'Tag'		, element : 'p', attributes : {'class' : 'tag'}},
	{ name : 'Cart'		, element : 'p', attributes : {'class' : 'cart'}},
	{ name : 'Home'		, element : 'p', attributes : {'class' : 'home'}},
	{ name : 'Key'		, element : 'p', attributes : {'class' : 'key'}},
	{ name : 'Lock'		, element : 'p', attributes : {'class' : 'lock'}},
	{ name : 'Image'	, element : 'p', attributes : {'class' : 'image'}},
	{ name : 'Video'	, element : 'p', attributes : {'class' : 'video'}},
	{ name : 'Code'		, element : 'code' },
	{ name : 'Arrow List'	, element : 'ul', attributes : {'class' : 'arrow-list'}}, 
	/*{ name : 'Star List'	, element : 'ul', attributes : {'class' : 'star-list'}},*/
	{ name : 'Check List'	, element : 'ul', attributes : {'class' : 'check-list'}}, 
	

	
]);
