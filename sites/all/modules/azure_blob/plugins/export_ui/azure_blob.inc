<?php

$plugin = array(
  'schema' => 'azure_blob',

  // The access permission to use
  'access' => 'administer site configuration',

  // You can actually define large chunks of the menu system here. Nothing
  // is required here. If you leave out the values, the prefix will default
  // to admin/structure and the item will default to the plugin name.
  'menu' => array(
    'menu prefix' => 'admin/config/media',
    'menu item' => 'azure-blob',
    // Title of the top level menu. Note this should not be translated,
    // as the menu system will translate it.
    'menu title' => 'Windows Azure Blob',
    // Description of the top level menu, which is usually needed for
    // menu items in an administration list. Will be translated
    // by the menu system.
    'menu description' => 'Administer Windows Azure Blob Storage configurations.',
  ),

  'title singular' => t('azure blob'),
  'title singular proper' => t('Windows Azure Blob'),
  'title plural' => t('azure blobs'),
  'title plural proper' => t('Windows Azure Blobs'),

  'form' => array(
    'settings' => 'azure_blob_export_ui_form',
    'validate' => 'azure_blob_export_ui_form_validate',
    'submit' => 'azure_blob_export_ui_form_submit',
  ),
);

function azure_blob_export_ui_form(&$form, &$form_state) {
  $form['info']['name']['#description'] = 
    t('The unique ID for this Azure storage. Will also be used as the URL scheme for the files.');

  $form['account'] = array(
    '#type' => 'textfield',
    '#title' => t('Account Name'),
    '#description' => t('The name of the Windows Azure Storage account.'),
    '#default_value' => $form_state['item']->account,
    '#required' => TRUE,
  );
  $form['primary_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Access key'),
    '#description' => t('The primary access key attached to this Windows Azure Storage account.'),
    '#default_value' => $form_state['item']->primary_key,
    '#required' => TRUE,
  );
  $form['blob_container'] = array(
    '#type' => 'textfield',
    '#title' => t('Blob container'),
    '#description' => t('The container attached to this Windows Azure Storage account.'),
    '#default_value' => $form_state['item']->blob_container,
    '#element_validate' => array('_azure_blob_container_validate'),
    '#required' => TRUE,
  );
  $form['cache_control'] = array(
    '#type' => 'textfield',
    '#title' => t('Cache Control'),
    '#description' => t('<p>The Cache-Control header to use when serving files. Default to <code>no-cache, must-revalidate, post-check=0, pre-check=0</code>.</p><p>Use something like <code>public, max-age=300</code> to allow Windows Azure CDN to serve older versions of the files.</p>'),
    '#default_value' => $form_state['item']->cache_control,
  );
}

/**
 * Check if container name is of the valid format.
 * 
 * 1. Container names must start with a letter or number, 
 *    and can contain only letters, numbers, and the dash (-) character.
 * 2. Every dash (-) character must be immediately preceded and followed by a 
 *    letter or number; consecutive dashes are not permitted in container names.
 * 3. All letters in a container name must be lowercase.
 * 4. Container names must be from 3 through 63 characters long.
 * 
 * @link http://msdn.microsoft.com/en-us/library/windowsazure/dd135715.aspx
 */
function _azure_blob_container_validate($element, &$form_state, $form) {
  if (!empty($element['#value']))
  {
    // TODO: Pattern does not match consecutive dashes
    $pattern_match = preg_match('/^[a-z0-9](([a-z0-9\-[^\-])){1,61}[a-z0-9]$/', $element['#value']);
    
    if ($pattern_match !== 1)
    {
      form_error($element, 
        t('Container names must start with a letter or number, and can contain only letters, numbers, and the dash (-) character.'));
    }
  }
}

function azure_blob_export_ui_form_validate(&$form, &$form_state) {
  // Validate the name is valid as a stream wrapper
  $name = $form_state['values']['name'];
  $pattern_match = preg_match('/^[a-z0-9-_]+$/', $name);
  
  if ($pattern_match !== 1) {
    form_error($form['info']['name'], 
      t('Only letters, figures and the underscore and dash signs are allowed in scheme names.'));
  }

  // When creating a new wrapper, validate that it does not override an existing wrapper
  if (empty($form_state['item']->id)) {
    $streams = stream_get_wrappers();
    if (in_array($name, $streams)) {
      form_error($form['info']['name'], t('This wrapper name already exists.'));
    }
  }
}

function azure_blob_export_ui_form_submit(&$form, &$form_state) {
  foreach (array('account', 'primary_key', 'blob_container', 'cache_control') as $key) {
    $form_state['item']->{$key} = $form_state['values'][$key];
  }
}
