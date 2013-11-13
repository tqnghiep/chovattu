<?php

function list_downloadable_per_user(){

    drupal_set_title(t('Downloadable list'));

    $form = array();
    $users = entity_load('user');

    $header = array(
        'uid' => t('ID'),
        'name' => t('Name'),
        'mail' => t('Mail'),
        'download_limit' => t('Maximum'),
        'downloaded' => t('Downloaded'),
        'downloadable' => t('Remaining'),
        'status' => t('Status'),
        'note' => t('Description'),
        'operation' => t('Operation'),
    );



    $options = array();
    foreach($users as $key => $user_obj){
        if(user_access('bypass private files download permission', $user_obj) && !in_array($user_obj->uid, array(0, 1))){

            $detail_path = 'admin/people/permissions/download/set';
            $detail_link = l(t('setting'), $detail_path, array('query' => array('uid'=>$user_obj->uid)));

            $context = get_setting_downloadable_data_per_user($user_obj->uid);
            $downloaded = download_count_per_user($user_obj);

            $status = $context['status'];

            $options[$user_obj->uid] = array(
                'uid' => $user_obj->uid,
                'name' => theme('username', array('account' => $user_obj)),
                'mail' => $user_obj->mail,
                'download_limit' => $context['download_limit'],
                'downloaded' => $downloaded['count'],
                'downloadable' => $context['download_limit'] - $downloaded['count'],
                'status' => $status ? t('Authenticated') : t('Expired'),
                'note' => $context['note'],
                'operation' => $detail_link,
            );
        }
    }

    $form['users'] = array(
        '#type' => 'tableselect',
        '#header' => $header,
        '#options' => $options,
        '#empty' => t('No content available.'),
        '#prefix' => '
        <h3><u>'.t('Notes').'<u>:</h3>
        <ul>
            <li><strong>'.t('Maximum').'</strong>: '.t('How many files which user had registered?').'</li>
            <li><strong>'.t('Downloaded').'</strong>: '.t('Total files which user has downloaded.').'</li>
            <li><strong>'.t('Remaining').'</strong>: '.t('Total files which user can download remaining.').'</li>
            <li><strong>'.t('Status').'</strong>: '.t('Authenticated: can download. Expired: cannot download').'</li>
            <li><strong>'.t('Description').'</strong>: '.t('It\'s additional info, example Company name, Telephone, Address, etc..').'</li>

        </ul>',
    );

    return $form;
}

function set_downloadable_per_user(){


    $form = array();

    if(isset($_GET['uid']) && is_numeric($_GET['uid'])){
        $user_obj = user_load($_GET['uid']);
        $context = get_setting_downloadable_data_per_user($user_obj->uid);

        drupal_set_title(t('Settings downloadable for ').$user_obj->name);

        $form['downloadable'] = array(
            '#type' => 'fieldset',
            '#title' => t('Settings downloadable for User'),
            '#collapsible' => FALSE,
            '#collapsed' => FALSE,

        );
        $form['downloadable']['uid'] = array('#type' => 'hidden', '#value' => $user_obj->uid);
        $form['downloadable']['display'] = array(
            '#type' => 'item',
            '#title' => t('Account'),
            '#markup' => $user_obj->name .' &lt;'. $user_obj->mail .'&gt;',
        );
        $form['downloadable']['download_limit'] = array(
            '#type' => 'textfield',
            '#title' => t('Download maximum limit'),
            '#default_value' => isset($context['download_limit']) ? $context['download_limit'] : 0,
            '#size' => 6,
        );
        $form['downloadable']['status'] = array(
            '#type' => 'radios',
            '#title' => t('Status'),
            '#options' => array(1=>t('Authenticated'), 0=>t('Expired')),
            '#default_value' => isset($context['status']) ? $context['status'] : 1,
        );
        $form['note'] = array(
            '#title' => t('Note'),
            '#type' => 'textarea',
            '#description' => t(''),
            '#default_value' => isset($context['note']) ? $context['note'] : '',
        );
    }else{
        drupal_set_title(t('Set message'));
        $form['downloadable_message'] = array(
            '#title' => t('Message'),
            '#type' => 'textarea',
            '#default_value' => variable_get('downloadable_message', ''),
            '#description' => t('This is information message to user which not yet register downloadable package or expired download.'),
        );
    }
    $form['submit'] = array(
        '#type' => 'submit', '#value' => t('Save'),
        '#suffix' => l(t('Cancel'), 'admin/people/permissions/download', array()),
    );
    return $form;
}

function set_downloadable_per_user_validate($form, &$form_state) {

}

function set_downloadable_per_user_submit($form, &$form_state) {

    if(isset($form_state['values']['uid'])){
        $_uid = $form_state['values']['uid'];
        $_limit = trim($form_state['values']['download_limit']);
        $_status = $form_state['values']['status'];
        $_note = $form_state['values']['note'];

        $data = array(
            'download_limit' => $_limit,
            'note'=>$_note,
            'status' => $_status
        );

        $exists = db_query('SELECT 1 FROM {user_downloadable} WHERE uid = :uid', array(':uid'=>$_uid))->fetchField();
        $path_redirect = '';
        if($exists){
            $update = db_update('user_downloadable')
                ->fields($data)
                ->condition('uid', $_uid)
                ->execute();
            if($update){
                $path_redirect = 'admin/people/permissions/download';
            }
        }else{
            $data['uid'] = $_uid;
            $insert = db_insert('user_downloadable')->fields($data)->execute();
            if($insert){
                $path_redirect = 'admin/people/permissions/download';
            }
        }
        if($path_redirect!=''){
            drupal_set_message(t('Successful'));
            $form_state['redirect'] = array(
                $path_redirect,
                array('query' => array('key' => 'value')),
                302,// $http_response_code
            );
        }
    }else{
        variable_set('downloadable_message', $form_state['values']['downloadable_message']);
        drupal_set_message('Successful');
    }
}