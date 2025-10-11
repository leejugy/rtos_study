#include "cli.h"
#include "usart.h"
#include "status.h"
#include "app_filex.h"
#include "que_ctl.h"
#include "sai.h"

static CLI_EXEC_RESULT cmd_help(cli_data_t *cli_data);
static CLI_EXEC_RESULT cmd_echo(cli_data_t *cli_data);
static CLI_EXEC_RESULT cmd_clear(cli_data_t *cli_data);
static CLI_EXEC_RESULT cmd_reboot(cli_data_t *cli_data);
static CLI_EXEC_RESULT cmd_dmesg(cli_data_t *cli_data);
static CLI_EXEC_RESULT cmd_read(cli_data_t *cli_data);
static CLI_EXEC_RESULT cmd_write(cli_data_t *cli_data);
static CLI_EXEC_RESULT cmd_touch(cli_data_t *cli_data);
static CLI_EXEC_RESULT cmd_mkdir(cli_data_t *cli_data);
static CLI_EXEC_RESULT cmd_remove(cli_data_t *cli_data);
static CLI_EXEC_RESULT cmd_aplay(cli_data_t *cli_data);
static CLI_EXEC_RESULT cmd_amixer(cli_data_t *cli_data);

cli_command_t cli_cmd[CMD_IDX_MAX] = 
{
    /* help */
    [CMD_HELP].help = \
    "help   : show command list\r\n",
    [CMD_HELP].name = "help",
    [CMD_HELP].func = cmd_help,
    [CMD_HELP].opt = "",
    [CMD_HELP].opt_size = 0,

    /* echo */
    [CMD_ECHO].help = \
    "echo   : printing message to terminal\r\n" \
    "<use>  : echo [argument to print]\r\n",
    [CMD_ECHO].name = "echo",
    [CMD_ECHO].func = cmd_echo,
    [CMD_ECHO].opt = "",
    [CMD_ECHO].opt_size = 0,

    /* clean */
    [CMD_CLEAR].help = \
    "clear  : clean the screen\r\n",
    [CMD_CLEAR].name = "clear",
    [CMD_CLEAR].func = cmd_clear,
    [CMD_CLEAR].opt = "",
    [CMD_CLEAR].opt_size = 0,

    /* reboot */
    [CMD_REBOOT].help = \
    "reboot  : restart the application\r\n",
    [CMD_REBOOT].name = "reboot",
    [CMD_REBOOT].func = cmd_reboot,
    [CMD_REBOOT].opt = "",
    [CMD_REBOOT].opt_size = 0,

    /* dmesg */
    [CMD_DMESG].help = \
    "dmesg      : turn on or off debug message\r\n" \
    "<use>      : dmesg <on/off>\r\n" \
    "<toggle>   : dmesg\r\n",
    [CMD_DMESG].name = "dmesg",
    [CMD_DMESG].func = cmd_dmesg,
    [CMD_DMESG].opt = "",
    [CMD_DMESG].opt_size = 0,

    /* write */
    [CMD_WRITE].help = \
    "write      : write file to route 128 bytes from the seek\r\n" \
    "<use>      : write -r <route> -s <seek> -w <string>\r\n",
    [CMD_WRITE].name = "write",
    [CMD_WRITE].func = cmd_write,
    [CMD_WRITE].opt = "rsw",
    [CMD_WRITE].opt_size = 3,

    /* read */
    [CMD_READ].help = \
    "read       : read file to route 128 bytes from the seek\r\n" \
    "<use>      : read -r <route> -s <seek>\r\n",
    [CMD_READ].name = "read",
    [CMD_READ].func = cmd_read,
    [CMD_READ].opt = "rs",
    [CMD_READ].opt_size = 2,

    /* touch */
    [CMD_TOUCH].help = \
    "touch      : create file to route\r\n" \
    "<use>      : touch <route>\r\n",
    [CMD_TOUCH].name = "touch",
    [CMD_TOUCH].func = cmd_touch,
    [CMD_TOUCH].opt = "",
    [CMD_TOUCH].opt_size = 0,

    /* mkdir */
    [CMD_MKDIR].help = \
    "mkdir      : create directory to route\r\n" \
    "<use>      : mkdir <route>\r\n",
    [CMD_MKDIR].name = "mkdir",
    [CMD_MKDIR].func = cmd_mkdir,
    [CMD_MKDIR].opt = "",
    [CMD_MKDIR].opt_size = 0,

    /* remove */
    [CMD_REMOVE].help = \
    "rm         : remove directory or file\r\n" \
    "<use>      : rm <route>\r\n",
    [CMD_REMOVE].name = "rm",
    [CMD_REMOVE].func = cmd_remove,
    [CMD_REMOVE].opt = "",
    [CMD_REMOVE].opt_size = 0,

    /* aplay */
    [CMD_APLAY].help = \
    "aplay      : play control wav file (44100hz, signed 16 only)\r\n" \
    "<play>     : aplay -l <route>\r\n"
    "<pause>    : aplay -p\r\n"
    "<stop>     : aplay -s\r\n" /* when stop music, can't resume music */
    "<resume>   : aplay -r\r\n",
    [CMD_APLAY].name = "aplay",
    [CMD_APLAY].func = cmd_aplay,
    [CMD_APLAY].opt = "lpsr",
    [CMD_APLAY].opt_size = 0,

    /* amixer */
    [CMD_AMIXER].help = \
    "amixer     : volume control device\r\n" \
    "<vol set>  : amixer -v <volume> [0 ~ 100]\r\n"
    "<vol up>   : amixer -u\r\n"
    "<vol down> : amixer -d\r\n",
    [CMD_AMIXER].name = "amixer",
    [CMD_AMIXER].func = cmd_amixer,
    [CMD_AMIXER].opt = "vud",
    [CMD_AMIXER].opt_size = 0,
};

/**
 * @brief Get command option. The command option is meaning -[character]
 * @note If you want to get argument you must set bit cli_arg->opt.get_ret
 * then you can get argument at cli_arg->arg
 * 
 * But if you don't want to get argument and only want to get result that
 * argument is exist, unset cli->opt.get_arg.
 * 
 * @param cli_data received command data
 * @param cli_arg has argument to be stored, and settngs to get argument
 * @return int 0 : argument exist (only retured at cli->opt.get_arg = 0)
 * 
 * int argument string length : when success to get argument
 * 
 * int -1 : fail to get argument
 */
static int cli_get_opt(cli_data_t *cli_data, cli_arg_t *cli_arg)
{
    char opt_want[] = {'-', cli_arg->cli_get.opt, '\0'};
    char *str = strstr(cli_data->cmd_str, opt_want);
    int idx = 0;
    int len = 0;
    int opt_len = strlen(opt_want);

    if (str == NULL)
    {
        return -1; /* no option detected */
    }

    if (!cli_arg->opt.get_ret)
    {
        return 0; /* if you don't want to use argument, use this. String must have option string like "-h" */
    }

    if (str[opt_len] == ' ')
    {
        opt_len++;
    }
    else
    {
        return -1; /* no argument input */
    }


    cli_arg->len = 0;

    str = str + opt_len;
    len = strlen(str);

    cli_arg->quotes = false;
    if(str[idx] == '\'' || str[idx] == '\"')
    {
        cli_arg->quotes = true;
        str++;
    }

    for (idx = 0; idx < len; idx++)
    {
        if (cli_arg->quotes)
        {
            if(str[idx] == '\'' || str[idx] == '\"')
            {
                break;
            }
            else if (str[idx] == '\0')
            {
                break;
            }
            else
            {
                cli_arg->arg[cli_arg->len++] = str[idx];
            }
        }
        else
        {
            if (str[idx] == '\0')
            {
                break;
            }
            else if(str[idx] == ' ')
            {
                break;
            }
            else
            {
                cli_arg->arg[cli_arg->len++] = str[idx];
            }
        }
    }
    cli_arg->arg[cli_arg->len] = 0;
    return cli_arg->len; /* option detect and store option argument to opt_str */
}

/**
 * @brief Get command arugment. The command arugment is next to the space character
 * @note If you want to get argument you must set bit cli_arg->opt.get_ret
 * then you can get argument at cli_arg->arg
 * 
 * But if you don't want to get argument and only want to get result that
 * argument is exist, unset cli->opt.get_arg.
 * 
 * @param cli_data received command data
 * @param cli_arg has argument to be stored, and settngs to get argument
 * @return int 0 : argument exist (only retured at cli->opt.get_arg = 0)
 * 
 * int argument string length : when success to get argument
 * 
 * int -1 : fail to get argument
 */
static int cli_get_arg(cli_data_t *cli_data, cli_arg_t *cli_arg)
{
    char *str = strchr(cli_data->cmd_str, ' ');
    int idx = 0;

    if (str == NULL)
    {
        return -1; /* Get argument failed, no argument after space */
    }

    if (!cli_arg->opt.get_ret)
    {
        return 0; /* just find argument */
    }
    
    for (idx = 0; idx < cli_arg->cli_get.num; idx++)
    {
        str = strchr(&str[1], ' ');
        if (str == NULL)
        {
            return -1; /* Get arg_num argument failed */
        }
    }
    
    str++;
    cli_arg->len = 0;
    int len = strlen(str);

    if((str[idx] == '\'' || str[idx] == '\"') && (cli_arg->quotes == false))
    {
        cli_arg->quotes = true;
        str++;
    }
    
    for (idx = 0; idx < len; idx++)
    {
        if (cli_arg->quotes)
        {
            if(str[idx] == '\'' || str[idx] == '\"')
            {
                break;
            }
            else if (str[idx] == '\0')
            {
                break;
            }
            else
            {
                cli_arg->arg[cli_arg->len++] = str[idx];
            }
        }
        else
        {
            if (str[idx] == '\0')
            {
                break;
            }
            else if(str[idx] == ' ')
            {
                break;
            }
            else
            {
                cli_arg->arg[cli_arg->len++] = str[idx];
            }
        }
    }
    cli_arg->arg[cli_arg->len] = 0;
    return cli_arg->len; /* Get argument success and copy argument to arg */
}

static int cli_cmp_cmd(char *rx, char *cmd_name)
{
    int idx = 0;

    for (idx = 0; idx < strlen(rx); idx++)
    {
        if (rx[idx] == 0 || rx [idx] == ' ')
        {
            break;
        }
        else if (rx[idx] != cmd_name[idx])
        {
            return -1; /* command is different */
        }
    }

    if (idx == strlen(cmd_name))
    {
        return 1; /* command is same */
    }

    return -1; /* this case is not exist */
}

static void reboot_deinit_apps()
{
}

static CLI_EXEC_RESULT cmd_reboot(cli_data_t *cli_data)
{
    /* address that pointing Reset_Handler function address's address */
    __IO void (**rst_func)() = (__IO void(**)())(BOOT_LOADER_ADD + 4);
    __IO uint32_t *iap_add = (__IO uint32_t *)BOOT_LOADER_ADD;

    print_dmesg("Reboot : check iap is available");

    /* check iap is valid */
    if ((*iap_add & 0x20000000) == 0x20000000)
    {
        print_dmesg("Reboot : iap is detected");
        /* waiting deinit apps */
        reboot_deinit_apps();
        /* set vector table to boot loader */
        SCB->VTOR = BOOT_LOADER_ADD;
        /* deinit all */
        HAL_RCC_DeInit();
        HAL_DeInit();
        HAL_ICACHE_DeInit();
        __set_MSP((__IO uint32_t)iap_add);
        __disable_irq();
        /* jump to bootloader's reset handler */
        (*rst_func)();
        return EXEC_RESULT_OK;
    }
    else
    {
        print_dmesg("Reboot : iap is not detected");
    }
    return EXEC_RESULT_ERR;
}

static CLI_EXEC_RESULT cmd_echo(cli_data_t *cli_data)
{
    int idx = 0;
    int len = 0;
    cli_arg_t cli_arg = {0, };

    cli_arg.opt.get_ret = true;
    while(1)
    {
        len = cli_get_arg(cli_data, &cli_arg);
        if (len > 0)
        {
            prints("%s ", cli_arg.arg);
            cli_arg.cli_get.num++;
            memset(cli_arg.arg, 0, sizeof(cli_arg.arg));
        }
        else
        {
            break;
        }
        if (cli_arg.quotes)
        {
            break;
        }
        idx++;
    }
    prints("\r\n");

    return EXEC_RESULT_OK;
}

static CLI_EXEC_RESULT cmd_dmesg(cli_data_t *cli_data)
{
    cli_arg_t cli_arg = {0, };
    char *msg = NULL;

    cli_arg.cli_get.num = 0;
    cli_arg.opt.get_ret = 1;

    if (cli_get_arg(cli_data, &cli_arg) < 0) /* toggle */
    {
        switch(status_get_int(STATUS_INTEGER_DMESG))
        {
            case STATUS_DMESG_ON:
                status_set_int(STATUS_INTEGER_DMESG, STATUS_DMESG_OFF);
                msg = "OFF";
                break;

            case STATUS_DMESG_OFF:
                status_set_int(STATUS_INTEGER_DMESG, STATUS_DMESG_ON);
                msg = "ON";
                break;
        }
    }
    else
    {
        if (strcmp(cli_arg.arg, "on") == 0)
        {
            status_set_int(STATUS_INTEGER_DMESG, STATUS_DMESG_ON);
            msg = "ON";
        }
        else if (strcmp(cli_arg.arg, "off") == 0)
        {
            status_set_int(STATUS_INTEGER_DMESG, STATUS_DMESG_OFF);
            msg = "OFF";
        }
        else
        {
            printr("unkown argument : %s", cli_arg.arg);
            return EXEC_RESULT_ERR;
        }
    }

    prints("debug message : %s\r\n", msg);
    return EXEC_RESULT_OK;
}

static CLI_EXEC_RESULT cmd_clear(cli_data_t *cli_data)
{
    prints("\x1B[2J");
    return EXEC_RESULT_OK;
}

static CLI_EXEC_RESULT cmd_help(cli_data_t *cli_data)
{
    int idx = 0;
    
    prints("===============[stm32 help cmd list]===============\r\n");
    for (idx = 0; idx < CMD_IDX_MAX; idx++)
    {
        prints("%s", cli_cmd[idx].help);
        if (idx != CMD_IDX_MAX - 1)
        {
            prints("\r\n");
        }
    }
    prints("===================================================\r\n");

    return EXEC_RESULT_OK;
}

static CLI_EXEC_RESULT cmd_read(cli_data_t *cli_data)
{
    sd_req_t sd_req = {0, };
    cli_arg_t cli_arg = {0, };
    int opt_idx = 0;
    int ret_size = 0;
    bool req_end = false;
    uint8_t buf[CMD_FILE_TRX_SIZE] = {0, };

    sd_req.buf_size = CMD_FILE_TRX_SIZE;
    sd_req.buf = buf;
    sd_req.req_end = &req_end;
    sd_req.req = SD_READ;
    sd_req.rd_size = &ret_size;

    cli_arg.cli_get.opt = cli_cmd[CMD_READ].opt[opt_idx++];
    cli_arg.opt.get_ret = 1;
    if (cli_get_opt(cli_data, &cli_arg) < 0)
    {
        printr("opt not detect : %c", cli_arg.cli_get.opt);
        return EXEC_RESULT_ERR;
    }
    strncpy(sd_req.route, cli_arg.arg, sizeof(sd_req.route));

    cli_arg.cli_get.opt = cli_cmd[CMD_READ].opt[opt_idx++];
    if (cli_get_opt(cli_data, &cli_arg) < 0)
    {
        printr("opt not detect : %c", cli_arg.cli_get.opt);
        return EXEC_RESULT_ERR;
    }
    sd_req.seek = atoi(cli_arg.arg);

    if (que_push(&sd_que, &sd_req, sizeof(sd_req)) < 0)
    {
        printr("push fail");
        return EXEC_RESULT_ERR;
    }

    sd_req_end_wait(&sd_req);
    prints("read : %s\r\n", sd_req.buf);
    prints("rd size : %d\r\n", *sd_req.rd_size);
    return EXEC_RESULT_OK;
}

static CLI_EXEC_RESULT cmd_write(cli_data_t *cli_data)
{
    sd_req_t sd_req = {0, };
    cli_arg_t cli_arg = {0, };
    int opt_idx = 0;
    bool req_end = false;
    uint8_t buf[CMD_FILE_TRX_SIZE] = {0, };

    sd_req.buf = buf;
    sd_req.req_end = &req_end;
    sd_req.req = SD_WRITE;

    cli_arg.cli_get.opt = cli_cmd[CMD_WRITE].opt[opt_idx++];
    cli_arg.opt.get_ret = 1;
    if (cli_get_opt(cli_data, &cli_arg) < 0)
    {
        printr("opt not detect : %c", cli_arg.cli_get.opt);
        return EXEC_RESULT_ERR;
    }
    strncpy(sd_req.route, cli_arg.arg, sizeof(sd_req.route));

    cli_arg.cli_get.opt = cli_cmd[CMD_WRITE].opt[opt_idx++];
    if (cli_get_opt(cli_data, &cli_arg) < 0)
    {
        printr("opt not detect : %c", cli_arg.cli_get.opt);
        return EXEC_RESULT_ERR;
    }
    sd_req.seek = atoi(cli_arg.arg);

    cli_arg.cli_get.opt = cli_cmd[CMD_WRITE].opt[opt_idx++];
    if (cli_get_opt(cli_data, &cli_arg) < 0)
    {
        printr("opt not detect : %c", cli_arg.cli_get.opt);
        return EXEC_RESULT_ERR;
    }
    strncpy((char *)sd_req.buf, cli_arg.arg, CMD_FILE_TRX_SIZE);
    sd_req.buf_size = strlen((char *)sd_req.buf);

    if (que_push(&sd_que, &sd_req, sizeof(sd_req)) < 0)
    {
        printr("push fail");
        return EXEC_RESULT_ERR;
    }

    sd_req_end_wait(&sd_req);
    prints("write : %s\r\n", sd_req.buf);
    return EXEC_RESULT_OK;
}

static CLI_EXEC_RESULT cmd_touch(cli_data_t *cli_data)
{
    sd_req_t sd_req = {0, };
    cli_arg_t cli_arg = {0, };
    bool req_end = false;

    sd_req.buf_size = CMD_FILE_TRX_SIZE;
    sd_req.buf = NULL;
    sd_req.req_end = &req_end;
    sd_req.req = SD_CREATE;

    cli_arg.cli_get.num = 0;
    cli_arg.opt.get_ret = 1;
    if (cli_get_arg(cli_data, &cli_arg) < 0)
    {
        printr("arg not detect");
        return EXEC_RESULT_ERR;
    }
    strncpy(sd_req.route, cli_arg.arg, sizeof(sd_req.route));

    if (que_push(&sd_que, &sd_req, sizeof(sd_req)) < 0)
    {
        printr("push fail");
        return EXEC_RESULT_ERR;
    }

    sd_req_end_wait(&sd_req);
    prints("touch : %s\r\n", sd_req.route);
    return EXEC_RESULT_OK;
}

static CLI_EXEC_RESULT cmd_mkdir(cli_data_t *cli_data)
{
    sd_req_t sd_req = {0, };
    cli_arg_t cli_arg = {0, };
    bool req_end = false;

    sd_req.buf_size = CMD_FILE_TRX_SIZE;
    sd_req.buf = NULL;
    sd_req.req_end = &req_end;
    sd_req.req = SD_MKDIR;

    cli_arg.cli_get.num = 0;
    cli_arg.opt.get_ret = 1;
    if (cli_get_arg(cli_data, &cli_arg) < 0)
    {
        printr("arg not detect");
        return EXEC_RESULT_ERR;
    }
    strncpy(sd_req.route, cli_arg.arg, sizeof(sd_req.route));

    if (que_push(&sd_que, &sd_req, sizeof(sd_req)) < 0)
    {
        printr("push fail");
        return EXEC_RESULT_ERR;
    }

    sd_req_end_wait(&sd_req);
    prints("mkdir : %s\r\n", sd_req.route);
    return EXEC_RESULT_OK;
}

static CLI_EXEC_RESULT cmd_remove(cli_data_t *cli_data)
{
    sd_req_t sd_req = {0, };
    cli_arg_t cli_arg = {0, };
    bool req_end = false;

    sd_req.buf_size = CMD_FILE_TRX_SIZE;
    sd_req.buf = NULL;
    sd_req.req_end = &req_end;
    sd_req.req = SD_REMOVE;

    cli_arg.cli_get.num = 0;
    cli_arg.opt.get_ret = 1;
    if (cli_get_arg(cli_data, &cli_arg) < 0)
    {
        printr("arg not detect");
        return EXEC_RESULT_ERR;
    }
    strncpy(sd_req.route, cli_arg.arg, sizeof(sd_req.route));

    if (que_push(&sd_que, &sd_req, sizeof(sd_req)) < 0)
    {
        printr("push fail");
        return EXEC_RESULT_ERR;
    }

    sd_req_end_wait(&sd_req);
    prints("rm : %s\r\n", sd_req.route);
    return EXEC_RESULT_OK;
}

static CLI_EXEC_RESULT cmd_aplay(cli_data_t *cli_data)
{
    sai_tx_req_t tx_req = {0, };
    cli_arg_t cli_arg = {0, };
    int idx = 0;    

    cli_arg.cli_get.opt = cli_cmd[CMD_APLAY].opt[idx++];
    cli_arg.opt.get_ret = 1;
    if (cli_get_opt(cli_data, &cli_arg) > 0)
    {
        prints("play wav file : %s\r\n", cli_arg.arg);
        tx_req.flag = SAI_NEW_MUSIC;    
        strcpy(tx_req.req.route, cli_arg.arg);
        goto push_out;
    }

    cli_arg.cli_get.opt = cli_cmd[CMD_APLAY].opt[idx++];
    cli_arg.opt.get_ret = 0;
    if (cli_get_opt(cli_data, &cli_arg) == 0)
    {
        prints("pause wav file\r\n", cli_arg.arg);
        tx_req.flag = SAI_PAUSE;    
        goto push_out;
    }

    cli_arg.cli_get.opt = cli_cmd[CMD_APLAY].opt[idx++];
    if (cli_get_opt(cli_data, &cli_arg) == 0)
    {
        prints("stop wav file\r\n", cli_arg.arg);
        tx_req.flag = SAI_STOP;    
        goto push_out;
    }

    cli_arg.cli_get.opt = cli_cmd[CMD_APLAY].opt[idx++];
    if (cli_get_opt(cli_data, &cli_arg) == 0)
    {
        prints("resume wav file\r\n", cli_arg.arg);
        tx_req.flag = SAI_RESUME;    
        goto push_out;
    }

    printr("no opt detected");
    return EXEC_RESULT_ERR;

push_out:
    if (que_push(&sai1_tx_que, &tx_req, sizeof(tx_req)) < 0)
    {
        printr("fail to push que");
        return EXEC_RESULT_ERR;
    }
    return EXEC_RESULT_OK;
}

static CLI_EXEC_RESULT cmd_amixer(cli_data_t *cli_data)
{
    sai_tx_req_t tx_req = {0, };
    cli_arg_t cli_arg = {0, };
    int idx = 0;
    uint8_t volume = 0;

    cli_arg.cli_get.opt = cli_cmd[CMD_AMIXER].opt[idx++];
    cli_arg.opt.get_ret = 1;
    if (cli_get_opt(cli_data, &cli_arg) > 0)
    {
        volume = atoi(cli_arg.arg);
        prints("amixer volume : %d\r\n", volume);
        tx_req.flag = SAI_VOL_CTL;
        if (volume < 0 || volume > 100)
        {
            printr("volume range");
            return EXEC_RESULT_ERR;
        }
        tx_req.req.volume = volume;
        goto push_out;
    }

    cli_arg.cli_get.opt = cli_cmd[CMD_AMIXER].opt[idx++];
    cli_arg.opt.get_ret = 0;
    if (cli_get_opt(cli_data, &cli_arg) == 0)
    {
        prints("up volume\r\n", cli_arg.arg);
        tx_req.flag = SAI_UP_VOL;    
        goto push_out;
    }

    cli_arg.cli_get.opt = cli_cmd[CMD_AMIXER].opt[idx++];
    if (cli_get_opt(cli_data, &cli_arg) == 0)
    {
        prints("dwon volume\r\n", cli_arg.arg);
        tx_req.flag = SAI_DOWN_VOL;    
        goto push_out;
    }

    printr("no opt detected");
    return EXEC_RESULT_ERR;

push_out:
    if (que_push(&sai1_tx_que, &tx_req, sizeof(tx_req)) < 0)
    {
        printr("fail to push que");
        return EXEC_RESULT_ERR;
    }
    return EXEC_RESULT_OK; 
}

static CLI_EXEC_RESULT __cli_work(char *rx, cli_data_t *cli_data)
{
    static int idx = 0;
    CLI_EXEC_RESULT exe_cmd = EXEC_RESULT_NO_CMD; 
    cli_arg_t cli_arg = {0, };

    /* command is still working */
    if (cli_data->work != CLI_WORK_NONE)
    {
        exe_cmd = cli_cmd[idx].func(cli_data);
        goto result_out;
    }
    else
    {
        /*
         * command is not working, set index zero 
         * to search command at bottom
         */
        idx = 0;
    }

    /* only input enter */
    if (strlen(rx) == 0)
    {
        exe_cmd = EXEC_NONE;
        goto result_out;
    }
    
    /* searching command at cli_cmd's member name */
    for (idx = 0; idx < CMD_IDX_MAX; idx++)
    {
        if (cli_cmp_cmd(rx, cli_cmd[idx].name) > 0)
        {
            cli_data->cmd_str = rx;
            cli_arg.opt.get_ret = false;
            cli_arg.cli_get.opt = 'h';
            /* check user input help option */
            if (cli_get_opt(cli_data, &cli_arg) != 0)
            {
                /* it hasn't help option */
                cli_data->opt_size = cli_cmd[idx].opt_size;
                strcpy(cli_data->opt, cli_cmd[idx].opt);
                exe_cmd = cli_cmd[idx].func(cli_data);
            }
            else
            {
                /* it has help option, print help */
                prints("%s", cli_cmd[idx].help);
                exe_cmd = EXEC_HELP;
            }
            break;
        }
    }

result_out:
    switch (exe_cmd)
    {
    /* command execution failed in some reason */
    case EXEC_RESULT_ERR:
        printr("[%s]execution failed : try chat help to use command", cli_cmd[idx].name);
        return exe_cmd;
    /* can't find any command at cli_cmd's member name */
    case EXEC_RESULT_NO_CMD:
        printr("unkwon command : %s", rx);
        return exe_cmd;
    /* just returning result */
    case EXEC_HELP:
    case EXEC_RESULT_OK:
    case EXEC_NONE:
    case EXEC_WAIT:
        return exe_cmd;
    /* unkown error */
    default:
        return EXEC_UNKOWN;
    }
}

static int cli_esc_work(char esc_chr, cli_work_t *cli_work)
{
    switch (esc_chr)
    {
    case CLI_ESC_MOVE_LEFT:
        if (cli_work->cur_pos <= 0)
        {
            goto out;
        }
        (cli_work->cur_pos)--;
        goto esc_out;
        
    case CLI_ESC_MOVE_RIGHT:
        if (cli_work->cur_pos >= cli_work->rx_cnt)
        {
            goto out;
        }
        (cli_work->cur_pos)++;
        goto esc_out;

    case CLI_ESC_MOVE_UP:
        if (cli_work->history_pos < cli_work->history_cnt)
        {
            cli_work->history_pos++;
        }
        goto history_out;

    case CLI_ESC_MOVE_DOWN:
        if (cli_work->history_pos > 1)
        {
            cli_work->history_pos--;
        }
        else if (cli_work->history_pos == 0)
        {
            goto out;
        }
        goto history_out;

    case CLI_ESC_SPECIAL:
        switch (cli_work->esc_num)
        {
        case '1': /* home key */
            cli_work->cur_pos = 0;
            goto mov_pos_out;

        case '4': /* end key */
            cli_work->cur_pos = cli_work->rx_cnt;
            goto mov_pos_out;

        default:
            return -1;
        }
        break;

    default:
        return -1;
    }

esc_out:
    prints("\x1b[%c", esc_chr);
    goto out;

mov_pos_out:
    prints("\x1b[%dG", 4 + cli_work->cur_pos); /* "~# " + cur_pos */
    goto out;

history_out:
    strcpy(cli_work->rx, cli_work->history[cli_work->history_pos - 1]);
    cli_work->cur_pos = strlen(cli_work->rx);
    cli_work->rx_cnt = strlen(cli_work->rx);
    prints("\x1b[0G~# \x1b[K%s", cli_work->rx);
    goto out;

out:
    return 1;
}

static void cli_del_str(cli_work_t *cli_work)
{
    int idx = 0;

    if (cli_work->cur_pos == 0)
    {
        return;
    }

    for (idx = cli_work->cur_pos; idx < cli_work->rx_cnt; idx++)
    {
        cli_work->rx[idx - 1] = cli_work->rx[idx];
    }
    cli_work->rx[idx - 1] = 0;
    cli_work->cur_pos--;
    cli_work->rx_cnt--;
}

static void cli_put_str(char put_str, cli_work_t *cli_work)
{
    int idx = (cli_work->rx_cnt > CMD_MAX_LEN - 1) ? CMD_MAX_LEN - 1 : cli_work->rx_cnt;

    for (; idx > cli_work->cur_pos; idx--)
    {
        cli_work->rx[idx] = cli_work->rx[idx - 1];
    }
    cli_work->rx[idx] = put_str;
    cli_work->cur_pos++;
    cli_work->rx_cnt++;
}

static void cli_history_insert(cli_work_t *cli_work)
{
    int idx = (cli_work->history_cnt > CLI_HISTORY_NUM - 1) ? CLI_HISTORY_NUM - 1 : cli_work->history_cnt;

    /* ignore case that enter only */
    if (strlen(cli_work->rx) == 0)
    {
        return;
    }

    for (; idx > 0 ; idx--)
    {
        strcpy(cli_work->history[idx], cli_work->history[idx - 1]);
    }

    strcpy(cli_work->history[0], cli_work->rx);

    if (cli_work->history_cnt < CLI_HISTORY_NUM)
    {
        cli_work->history_cnt++;
    }
    cli_work->history_pos = 0;
}

CLI_STATUS cli_work(char *rx)
{
    static cli_work_t cli_work = {0, };
    static cli_data_t cli_data = {0, };
    static int esc_cnt = 0;
    static bool esc_seq = 0;

    int len = strlen(rx);
    int idx = 0;
    int ret = CLI_NONE;

    switch (cli_data.work)
    {
    /* when CTRL + C input. */        
    case CLI_WORK_STOP: 
    /* command execution end */
    case CLI_WORK_END:
        goto exec_cmd;
    /* command must need to be worked continuously. */
    case CLI_WORK_CONTINUE: 
        /* when CTRL + C input. */
        if (strchr(rx, 0x03))
        {
            /* set cli work stop it will make stop command execution */
            cli_data.work = CLI_WORK_STOP;
            prints("^C\r\n");
        }
        goto exec_cmd;
    
    default:
    case CLI_WORK_NONE:
        break;
    }
    
    for (idx = 0; idx < len; idx++)
    {
        /* 
         * escape sequence like Home, End, right arrow left arrow 
         * up, down arrow.
         */
        if (esc_seq)
        {
            esc_cnt++;
            /* parse escape sequence character */
            switch (esc_cnt)
            {
            case 2:
                /* check invalid escape sequence format */
                if (rx[idx] != '[')
                {
                    esc_cnt = 0;
                    esc_seq = false;
                }
                ret = CLI_ESCAPE_SEQ;
                break;

            case 3:
                /* if fail to check escape sequence */
                if (cli_esc_work(rx[idx], &cli_work) > 0)
                {
                    esc_cnt = 0;
                    esc_seq = false;
                }
                /* 
                 * check the escape sequence's number to get home and end 
                 * home -> 0x1b, '[', '1', '~'
                 * end -> 0x1b, '[', '4', '~'
                 */
                else if ('0' <= rx[idx] || rx[idx] <= '9')
                {
                    cli_work.esc_num = rx[idx];
                }
                else
                {
                    esc_cnt = 0;
                    esc_seq = false;
                }
                ret = CLI_ESCAPE_SEQ;
                break;

            case 4:
                cli_esc_work(rx[idx], &cli_work);
                esc_cnt = 0;
                esc_seq = false;
                ret = CLI_ESCAPE_SEQ;
                break;
            }
        }
        /* CTRL + C */
        else if (rx[idx] == 0x03)
        {
            prints("^C\r\n");
            ret = CLI_CANCEL;
        }
        /* enter -> command execute */
        else if (rx[idx] == '\n' || rx[idx] == '\r')
        {
            prints("\r\n");
            cli_history_insert(&cli_work);
            goto exec_cmd;
        }
        /* delete string */
        else if (rx[idx] == '\b')
        {
            if (cli_work.rx_cnt > 0 && cli_work.cur_pos > 0)
            {
                cli_del_str(&cli_work);
                prints("\b\x1b[P");
            }
            ret = CLI_ESCAPE_SEQ;
        }
        /* escape sequence detected */
        else if (rx[idx] == '\x1b')
        {
            esc_cnt = 1;
            esc_seq = true;
            ret = CLI_ESCAPE_SEQ;
        }
        /* put string */
        else
        {
            /* If string length is 512, It will be cause of overflow at cli_history_insert.
             * cli_history_insert use strcpy to copy command to history. If string length is
             * 512, strcpy try to copy 513 byte by including null character
             */
            if (cli_work.rx_cnt < CMD_MAX_LEN - 1)
            {
                cli_put_str(rx[idx], &cli_work);
                prints("\x1b[@%c", rx[idx]);
                ret = CLI_INPUT;
            }
            else
            {
                printr("too many string");
                ret = CLI_ERR;
            }
        }
    }
    return ret;
exec_cmd:
    CLI_EXEC_RESULT cli_ret = __cli_work(cli_work.rx, &cli_data);
    switch (cli_ret)
    {
    /* command execution end */
    case EXEC_RESULT_ERR:
    case EXEC_RESULT_NO_CMD:
    case EXEC_HELP:
    case EXEC_RESULT_OK:
        ret = CLI_EXEC_CMD;
        break;
    /* you input only enter key */
    case EXEC_NONE:
        ret = CLI_ENTER;
        break;
    /* wait or error, do noting */
    default:
    case EXEC_WAIT:
        ret = CLI_NONE; 
        break;
    }
    memset(cli_work.rx, 0, sizeof(cli_work.rx));
    cli_work.rx_cnt = 0;
    cli_work.cur_pos = 0;
    return ret;
}

void __cli_proc(char *rx)
{
    if (strlen(rx) == 0)
    {
        return;
    }

    switch (cli_work((char *)rx))
    {
    /* command is executed */
    case CLI_EXEC_CMD:
    /* only enter input */
    case CLI_ENTER:
    /* cancel command */
    case CLI_CANCEL:
        prints("~# ");
        break;

    case CLI_NONE:
    case CLI_INPUT:
    case CLI_ESCAPE_SEQ:
    case CLI_ERR:
    default:
        break;
    }
}

static int system_read(char *buf, size_t buf_size)
{
    return 1;
}

int	system(const char *__string)
{
    /* TODO use list to find result of system command */
    return 1;
}

void cli_proc()
{
    uint8_t rx_buf[UART_TRX_BUF_SIZE] = {0, };

    if (uart_recv(UART1_IDX, rx_buf, sizeof(rx_buf)) > 0)
    {
        __cli_proc((char *)rx_buf);
    }

    memset(rx_buf, 0, sizeof(rx_buf));
    if (system_read((char *)rx_buf, sizeof(rx_buf)) > 0)
    {
        __cli_proc((char *)rx_buf);
    }
}