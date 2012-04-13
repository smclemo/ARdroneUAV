/**
 *  \brief    VP Api. Composite Stage Declaration
 *  \author   Sylvain Gaeremynck <sylvain.gaeremynck@parrot.fr>
 *  \author   Aurelien Morelle <aurelien.morelle@parrot.fr>
 *  \author   Thomas Landais <thomas.landais@parrot.fr>
 *  \version  1.0
 *  \date     first release 21/03/2007
 */

#include <VP_Api/vp_api_io_multi_stage.h>
#include <VP_Api/vp_api_error.h>
#include <VP_Os/vp_os_assert.h>

C_RESULT
vp_api_multi_stage_open(vp_api_io_multi_stage_config_t *cfg)
{
  uint32_t i;

  for(i = 0; i < cfg->nb_stages; i++)
  {
    VP_OS_ASSERT(cfg->stages[i].funcs.open);
    VP_OS_ASSERT(cfg->stages[i].funcs.transform);
    VP_OS_ASSERT(cfg->stages[i].funcs.close);

    if(FAILED(cfg->stages[i].funcs.open(cfg->stages[i].cfg)))
      return (FAIL);
  }

  return (SUCCESS);
}

C_RESULT
vp_api_multi_stage_transform(vp_api_io_multi_stage_config_t *cfg, vp_api_io_data_t *in, vp_api_io_data_t *out)
{
  uint32_t i;
  C_RESULT res = (FAIL);

  // if a stage is selected then execute only this one
  if(cfg->activ_stage >= 0 && cfg->activ_stage < cfg->nb_stages)
    res = cfg->stages[cfg->activ_stage].funcs.transform(cfg->stages[cfg->activ_stage].cfg, in, out);

  // Otherwise if activ_stage == -1 then execute all stages
  if(cfg->activ_stage == VP_API_EXECUTE_ALL_STAGES)
  {
    res = SUCCESS;
    for(i = 0; i < cfg->nb_stages && res == SUCCESS; i++)
    {
      res = cfg->stages[i].funcs.transform(cfg->stages[i].cfg, in, out);
    }
  }

  if(cfg->activ_stage == VP_API_EXECUTE_NO_STAGE)
    res = SUCCESS;

  return res;
}

C_RESULT
vp_api_multi_stage_close(vp_api_io_multi_stage_config_t *cfg)
{
  uint32_t i;

  for(i = 0; i < cfg->nb_stages; i++)
  {
    if(FAILED(cfg->stages[i].funcs.close(cfg->stages[i].cfg)))
      return (FAIL);
  }

  return (SUCCESS);
}
