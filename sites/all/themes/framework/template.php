<?php
// $Id: template.php,v 1.4.2.6 2011/02/18 05:26:30 andregriffin Exp $

/**
 * Implements hook_html_head_alter().
 * We are overwriting the default meta character type tag with HTML5 version.
 */
function framework_html_head_alter(&$head_elements) {
  $head_elements['system_meta_content_type']['#attributes'] = array(
    'charset' => 'utf-8'
  );
}
/*This function allows other page templates to work.
 *Example: If I want to create a new template page called page--3col.tpl.php. I just need to create the template and it will work.
 */
//function framework_preprocess_page(&$vars, $hook) {
// The body tag's classes are controlled by the $classes_array variable. To
// remove a class from $classes_array, use array_diff().
    //if (isset($vars['node'])) {
        //if ($vars['node']->type == 'page1column') {
            //$vars['classes_array'] = array_diff($vars['classes_array'], array('sidebar-second'));
        //}
    //}
//}
  
//function framework_preprocess_html(&$vars, $hook) {
    // page
    // Renders a new page template to the list of templates used if it exists
    //if (isset($vars['node'])) {
        //if ($vars['node']->type == 'page1column') {
            //unset($vars['classes_array'][] = 'sidebar-first');
            //$vars['classes_array'] = array_diff($vars['classes_array'], array('sidebar-second'));
            //unset($vars['sidebar-second']['sidebar-first']['one-sidebar']['two-sidebar']);
            /*$vars['sidebar_second'] = '';
            $vars['sidebar_first'] = '';
            if (($pos = array_search('one-sidebar', $vars['classes_array'])) !== false) { unset($vars['classes_array'][$pos]); }
            if (($pos = array_search('two-sidebar', $vars['classes_array'])) !== false) { unset($vars['classes_array'][$pos]); }
            if (($pos = array_search('sidebar-second', $vars['classes_array'])) !== false) { unset($vars['classes_array'][$pos]); }
            if (($pos = array_search('sidebar-first', $vars['classes_array'])) !== false) { unset($vars['classes_array'][$pos]); }*/
            //$vars['classes_array'][] = 'no-sidebars';
            //$vars['classes_array'][] = 'and-another';
        //}
    //}
//}

/**
 * Return a themed breadcrumb trail.
 *
 * @param $breadcrumb
 *   An array containing the breadcrumb links.
 * @return a string containing the breadcrumb output.
 */
function framework_breadcrumb($variables) {
  $breadcrumb = $variables['breadcrumb'];

  if (!empty($breadcrumb)) {
    $output = '<section id="breadcrumbs" role="breadcrumbs" class="clearfix">';
	$output .= '<nav class="breadcrumb">' . implode(' ', $breadcrumb) . '</nav>';
	$output .= '</section>';
	// Provide a navigational heading to give context for breadcrumb links to
    // screen-reader users. Make the heading invisible with .element-invisible.
    //$output = '<h2 class="element-invisible">' . t('You are here') . '</h2>';
    // Uncomment to add current page to breadcrumb
	// $breadcrumb[] = drupal_get_title();
    //$output .= '<nav class="breadcrumb">' . implode(' ', $breadcrumb) . '</nav>';
	//return '<nav class="breadcrumb">' . implode(' ', $breadcrumb) . '</nav>'; //This worked
    return $output;
  }
}
    
function framework_preprocess_page(&$vars, $hook) {
    if (isset($vars['node'])) {
        $vars['theme_hook_suggestions'][] = 'page__' . $vars['node']->type;
        if (!empty($vars['node']) && $vars['node']->type == 'page1column') {
            unset($vars['sidebar-second']['sidebar-first']['one-sidebar']['two-sidebar']);
            $vars['classes_array'] = array_diff($vars['classes_array'], array('sidebar-second'));
        }
    }
    /*if ($vars['node']->type == 'page1column') {
        $vars['classes_array'] = array_diff($vars['classes_array'], array('sidebar-second'));
    }*/
    if (isset($vars['main_menu'])) {
    $vars['main_menu'] = theme('links__system_main_menu', array(
      'links' => $vars['main_menu'],
      'attributes' => array(
        'class' => array('links', 'main-menu', 'clearfix'),
      ),
      'heading' => array(
        'text' => t('Main menu'),
        'level' => 'h2',
        'class' => array('element-invisible'),
      )
    ));
  }
  else {
    $vars['main_menu'] = FALSE;
  }
  if (isset($vars['secondary_menu'])) {
    $vars['secondary_menu'] = theme('links__system_secondary_menu', array(
      'links' => $vars['secondary_menu'],
      'attributes' => array(
        'class' => array('links', 'secondary-menu', 'clearfix'),
      ),
      'heading' => array(
        'text' => t('Secondary menu'),
        'level' => 'h2',
        'class' => array('element-invisible'),
      )
    ));
  }
  else {
    $vars['secondary_menu'] = FALSE;
  }
    drupal_add_js(drupal_get_path('theme', 'framework') .'/styleswitcher.js', 'file');
}

/**
 * Duplicate of theme_menu_local_tasks() but adds clearfix to tabs.
 */
function framework_menu_local_tasks(&$vars) {
  $output = '';

  if (!empty($vars['primary'])) {
    $vars['primary']['#prefix'] = '<h2 class="element-invisible">' . t('Primary tabs') . '</h2>';
    $vars['primary']['#prefix'] .= '<ul class="tabs primary clearfix">';
    $vars['primary']['#suffix'] = '</ul>';
    $output .= drupal_render($vars['primary']);
  }
  if (!empty($vars['secondary'])) {
    $vars['secondary']['#prefix'] = '<h2 class="element-invisible">' . t('Secondary tabs') . '</h2>';
    $vars['secondary']['#prefix'] .= '<ul class="tabs secondary clearfix">';
    $vars['secondary']['#suffix'] = '</ul>';
    $output .= drupal_render($vars['secondary']);
  }
  return $output;

}

/**
 * Override or insert variables into the node template.
 */
function framework_preprocess_node(&$variables) {
  $variables['submitted'] = t('Published by !username on !datetime', array('!username' => $variables['name'], '!datetime' => $variables['date']));
  if ($variables['view_mode'] == 'full' && node_is_page($variables['node'])) {
    $variables['classes_array'][] = 'node-full';
  }
  //Removing the Read more linke and placing it in a different place
  // Let's get that read more link out of the generated links variable!
  unset($variables['content']['links']['node']['#links']['node-readmore']['sidebar-second']['sidebar-first']['one-sidebar']['two-sidebar']);
   // Now let's put it back as it's own variable! So it's actually versatile!
    $variables['newreadmore'] = t('<span class="newreadmore"> <a href="!title">...Read More</a> </span>',
      array(
        '!title' => $variables['node_url'],
      )
   );
}

/**
 * Changes the search form to use the "search" input element of HTML5.
 
function framework_preprocess_search_block_form(&$vars) {
  $vars['search_form'] = str_replace('type="text"', 'type="search"', $vars['search_form']);
}*/
function framework_form_alter(&$form, &$form_state, $form_id) {
  if ($form_id == 'search_block_form') {
    $form['search_block_form']['#title'] = t('Search'); // Change the text on the label element
    $form['search_block_form']['#title_display'] = 'invisible'; // Toggle label visibilty
    $form['search_block_form']['#size'] = 30;  // define size of the textfield
    $form['search_block_form']['#default_value'] = t('Search'); // Set a default value for the textfield
    $form['actions']['submit']['#value'] = t('GO!'); // Change the text on the submit button
    $form['actions']['submit'] = array('#type' => 'image_button', '#src' => base_path() . path_to_theme() . '/images/bt-search-bg.png');

// Add extra attributes to the text box
    $form['search_block_form']['#attributes']['onblur'] = "if (this.value == '') {this.value = 'Search';}";
    $form['search_block_form']['#attributes']['onfocus'] = "if (this.value == 'Search') {this.value = '';}";
  }
}
?>
