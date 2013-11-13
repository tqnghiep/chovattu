<?php


function user_files_download_report($obj_user){

    drupal_set_title($obj_user->name.t(' download dashboard'));

    $str_sql = "SELECT n.nid,
                    n.title,
                    fm.fid,
                    fm.filename,
                    fm.filemime,
                    fm.filesize,
                    fdfd.field_documents_description description,
                    tbl_counter.counter
                FROM field_data_field_documents fdfd
                    INNER JOIN (
                        SELECT tdfp.fid, COUNT(*) AS counter
                        FROM track_da_files tdf
                            INNER JOIN track_da_files_paths tdfp ON tdf.pid=tdfp.pid
                        WHERE tdf.uid=:uid
                        GROUP BY tdfp.fid
                    ) AS tbl_counter ON fdfd.field_documents_fid=tbl_counter.fid
                    INNER JOIN file_managed fm ON fm.fid = fdfd.field_documents_fid
                    INNER JOIN node n ON n.nid=fdfd.entity_id ";
    $query = db_query($str_sql, array(':uid'=>$obj_user->uid));
    $rows = array();
    foreach ($query as $record) {

        $detail_path = 'user/'.$obj_user->uid.'/download/detail';
        $detail_link = l(t('Details'), $detail_path, array('attributes' => array('class' => array('detail-link')), 'query' => array('fid'=>$record->fid)));

        $rows [] = array(
            'data' => array(
                array('data' => $record->nid, 'class' => 'nid'),
                array('data' => $record->title, 'class' => 'title'),
                array('data' => $record->filename, 'class' => 'filename'),
                array('data' => $record->filemime, 'class' => 'filemime'),
                array('data' => format_size($record->filesize), 'class' => 'filesize'),
                array('data' => $record->description, 'class' => 'description'),
                array('data' => $record->counter, 'class' => 'counter'),
                array('data' => $detail_link, 'class' => 'options')
            ),
            'class' => array('file-download-item',)
        );
    }
    $header = array(
        t('ID'),
        t('Name'),
        t('Book File'),
        t('Type'),
        t('Size'),
        t('Description'),
        t('Counter'),
        t('Options')
    );
    $html_output = theme('table', array( 'header' => $header, 'rows'=> $rows, 'attributes' => array('class' => array('file-download-report'))));
    return $html_output;
}




function user_files_download_report_detail($obj_user){

    if(isset($_GET['fid']) && is_numeric($_GET['fid'])){

        drupal_set_title(t('Detail download of the file '));

        $fid = $_GET['fid'];
        $str_sql = "SELECT tdf.time,
                        tdf.ip,
                        tdf.browser,
                        tdf.browser_platform,
                        tdf.referer
                    FROM track_da_files tdf
                        INNER JOIN track_da_files_paths tdfp ON tdf.pid=tdfp.pid
                    WHERE tdf.uid = :uid AND tdfp.fid = :fid
                    ORDER BY tdf.time ";
        $query = db_query($str_sql, array(':uid'=>$obj_user->uid, ':fid'=>$fid));
        $rows = array();
        foreach ($query as $record) {

            $rows [] = array(
                'data' => array(
                    array('data' => date('d F Y h:i s', $record->time), 'class' => 'time'),
                    array('data' => $record->ip, 'class' => 'ip'),
                    array('data' => $record->browser, 'class' => 'browser'),
                    array('data' => $record->browser_platform, 'class' => 'browser-platform'),
                    array('data' => $record->referer, 'class' => 'referer'),
                    array('data' => '<a href="'.$record->referer.'" target="_blank">view</a>', 'class' => 'option')
                ),
                'class' => array('row-file-download-item',)
            );
        }
        $header = array(
            t('Date'),
            t('from IP'),
            t('by Browser'),
            t('Browser Platform'),
            t('from Link'),
            t('Option')
        );
        $html_output = theme('table', array( 'header' => $header, 'rows'=> $rows, 'attributes' => array('class' => array('file-download-report'))));
        return $html_output;
    }else{
        return t('Missing file id argument. <a href="@url">Click here</a>  to back Download Dashboard',
                    array(
                        '@url'=>url('user/'.$obj_user->uid.'/download')
                    )
        );
    }
}



function user_files_download_your_package($obj_user){

    drupal_set_title(t('Your package '));

    $header = array(
        'uid' => t('ID'),
        'name' => t('Name'),
        'mail' => t('Mail'),
        'download_limit' => t('Maximum'),
        'downloaded' => t('Downloaded'),
        'downloadable' => t('Remaining'),
        'status' => t('Status'),
    );

    $context = get_setting_downloadable_data_per_user($obj_user->uid);
    $downloaded = download_count_per_user($obj_user);
    $status = $context['status'];

    $rows [] = array(
        'data' => array(
            array('data' => $obj_user->uid, 'class' => 'uid'),
            array('data' => theme('username', array('account' => $obj_user)), 'class' => 'name'),
            array('data' => $obj_user->mail, 'class' => 'mail'),
            array('data' => $context['download_limit'], 'class' => 'download-limit'),
            array('data' => $downloaded['count'], 'class' => 'downloaded'),
            array('data' => $context['download_limit'] - $downloaded['count'], 'class' => 'downloadable'),
            array('data' => $status ? t('Authenticated') : t('Expired'), 'class' => 'status'),
        ),
        'class' => array('')
    );
    $str_description = '
    <h3><u>'.t('Notes').'<u>:</h3>
    <ul>
        <li><strong>'.t('Maximum').'</strong>: '.t('How many files which user had registered?').'</li>
        <li><strong>'.t('Downloaded').'</strong>: '.t('Total files which user has downloaded.').'</li>
        <li><strong>'.t('Remaining').'</strong>: '.t('Total files which user can download remaining').'</li>
        <li><strong>'.t('Status').'</strong>: '.t('Authenticated: can download. Expired: cannot download').'</li>

    </ul>';

    $package = theme('table', array( 'header' => $header, 'rows'=> $rows, 'attributes' => array('class' => array(''))));



    return $package.$str_description;

}



function message_user_cannot_download(){

    return variable_get('downloadable_message', '');
}