#!/usr/bin/env python3

from subprocess import call, Popen, PIPE
import argparse
import os

versionOpts = [8, 10, 11]
sizePolicyOpts = ['Inclusive16BitWords', 'Inclusive32BitWords', 
                  'Inclusive32BitBytes', 'Exclusive16BitWords']




def set_up_parser():
  """Initialize a command line parser and return it"""

  parser = argparse.ArgumentParser(description='Convert a file between two data formats')
  parser.add_argument('-i', '--inputformat', type=int, dest='inputformat', 
                      choices=versionOpts, help='Version of input data')
  parser.add_argument('-o', '--outputformat', type=int, dest='outputformat', 
                      choices=versionOpts, help='Version of output data')
  parser.add_argument('-s', '--source', dest='source', 
                      help='Data source url (stdin="-", file=file:///path/to/data, ring=tcp://host/ringname)')
  parser.add_argument('-d', '--sink', dest='sink', 
                      help='Data sink url (stdout="-", file=file:///path/to/data, ring=tcp://host/ringname)')
  parser.add_argument('-b', '--v8-buffer-size', dest='v8buffersize', 
                      type=int, default=8192,
                      help='Size of version 8 buffer') 
  parser.add_argument('-p', '--v8-size-policy', dest='v8sizepolicy', 
                      choices=sizePolicyOpts, 
                      default=sizePolicyOpts[0],
                      help='Policy for how to interpret physics event sizes')
  return parser





def compute_conversion_stages(input, output):
  """Populate a list of versions for each conversion stages

  If a conversion between two formats requires a multi-stage conversion,
  then a pipeline of format_converter programs will be created that
  incrementally converts the data between adjacent formats. Each stage of the pipeline
  must know an input version and an output version. This creates a list of
  dictionaries, where each dictionary contains the input version and output
  version of each stage.
  """

  inIndex = versionOpts.index(input)
  outIndex = versionOpts.index(output)
  
  stages = []
  if outIndex > inIndex :
    for index in range(inIndex, outIndex, 1):
      stages.append({'in':versionOpts[index], 'out':versionOpts[index+1]})
  else:
    for index in range(inIndex, outIndex, -1):
      stages.append({'in':versionOpts[index], 'out':versionOpts[index-1]})

  return stages




def compute_single_stage_options(stage, args):
  """Returns a dict of options for a given pipeline stage """
  
  cmd = {'--input-version' : stage['in'], 
         '--output-version': stage['out'], 
         '--source'        : args.source, 
         '--sink'          : args.sink}

  if (stage['in'] == 8) or (stage['out'] == 8):
    cmd['--v8-buffer-size'] = args.v8buffersize
  if stage['in'] == 8:
    cmd['--v8-size-policy'] = args.v8sizepolicy 

  return cmd


def compute_compound_stages_options(stages, args):
  """Returns a list of dicts, each dict contains options for a pipeline stage """
  
  stage_options = []
  for stage in stages:
    stage_options.append(compute_single_stage_options(stage, args))

  stage_options[0]['--sink']  = '-'
  stage_options[-1]['--source'] = '-'
  for stage in stages[1:-2]:
    stage['--source'] = '-'
    stage['--sink'] = '-';
    
  return stage_options


def compute_stage_options(stages, args):
  """Dispatch to stage option computer """

  if len(stages) > 1:
    return compute_compound_stages_options(stages, args)
  else:
    return [compute_single_stage_options(stages[0], args)]


def executable():
  """Returns the path of the format_converter executable """

  bin_dir = os.path.dirname(os.path.realpath(__file__))
  return os.path.join(bin_dir, 'format_converter')
  

def set_up_simple_command(stage_opts):
  opts = []
  opts.append(executable())
  for (key, val) in stage_opts.items():
    opts.append( '{0}={1}'.format(key, val) )

  
  return opts
  

def run_simple_conversion(stage_opts):
  """Sets up and runs a single instance of format_converter """

  cmd = set_up_simple_command(stage_opts)

  call(cmd, shell=False)



def run_compound_conversion(multi_stage_opts):
  """Sets up and runs a pipeline of format_converter programs

  The first and last stages of the pipeline are unique because
  they must take the source and sink arguments appropriately
  from the command line. The middle stages however all are the same.
  This wait till the last stage of the pipeline has exited.
  """

  try:
    p = []
    p.append(Popen(set_up_simple_command(multi_stage_opts[0]), stdout=PIPE))

    for stage_opts in multi_stage_opts[1:-2]:
      p.append(Popen(set_up_simple_command(stage_opts), 
                     stdin=p[-1].stdout, stdout=PIPE))

    p.append(Popen(set_up_simple_command(multi_stage_opts[-1]), 
                    stdin=p[-1].stdout))

    p[-1].communicate()[0]

    print('Successfully converted')

  except OSError as e:
    print('OSError! ', e.child_traceback)




def build_pipeline(args):
  """Calls the appropriate function to handle the conversion """

  stages = compute_conversion_stages(args.inputformat, args.outputformat)
  stage_opts = compute_stage_options(stages, args)
  
  if len(stages) == 1:
    run_simple_conversion(stage_opts[0])
  elif len(stages) > 1:
    run_compound_conversion(stage_opts)
  else:
    raise RuntimeError()





def main() :
  """Main method """

  parser = set_up_parser()
  args = parser.parse_args()
  pipeline = build_pipeline(args)



if __name__ == '__main__' :
    main()
